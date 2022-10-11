#include "core/core.h"
#include "tiny_stl/stack.hpp"
#include "tiny_stl/algorithm.hpp"
#include "data_binify/data_binify.h"
#include "data_utils/lz4.h"
#include "resource_bundle/resource_bundle.h"
#include "resource_bundle_writer/resource_bundle_writer.hpp"
#include "data_binify/write_helper.hpp"
#include "dbg/print.h"

namespace Binny {
	using namespace Binify;

	static char const * idToString(uint32_t id_) {
		static char txt[5];
		txt[0] = (char)(id_ >> 0);
		txt[1] = (char)(id_ >> 8);
		txt[2] = (char)(id_ >> 16);
		txt[3] = (char)(id_ >> 24);
		txt[4] = 0;
		return txt;
	}

	BundleWriter::BundleWriter(int addressLength_, Memory_Allocator* allocator_) :
			addressLength(addressLength_),
			allocator(allocator_),
			chunkRegistry(allocator_),
			o(allocator_)
	{
		assert(addressLength == 64 || addressLength == 32);
		o.setAddressLength(addressLength);
	}

	bool BundleWriter::registerChunk(uint32_t id_,
	                                 uint8_t version_,
	                                 tiny_stl::vector<uint32_t> const& dependencies_,
																	 ChunkWriter const & setup_,
	                                 ChunkWriter const & item_) {
		if(chunkRegistry.find(id_) != chunkRegistry.end()) return false;

		DirEntryWriter entry = {
				id_,
				version_,
				dependencies_,
				setup_,
				item_,
				tiny_stl::vector<DirNamePair>{ allocator }
		};

		chunkRegistry.insert(tiny_stl::pair<uint32_t, DirEntryWriter>(id_,entry));
		return true;
	}

	void BundleWriter::addItemToChunk(uint32_t id_, tiny_stl::string const & name_, void* item_) {
		if(chunkRegistry.find(id_) == chunkRegistry.end()) {
			debug_printf("Chunk %i not in bundle registery\n", id_);
			assert( chunkRegistry.find( id_ ) != chunkRegistry.end());
		}
		chunkRegistry.find(id_)->second.items.push_back(tiny_stl::pair(name_, item_));
	}

	bool BundleWriter::build(VFile_Handle result_)
	{
		ResourceBundle_Header bundleHeader;
		// write a dummy header, will we fill properly after compression
		VFile_Write(result_, &bundleHeader, sizeof(bundleHeader));

		// mark beginning of the main block
		o.reserveLabel("begin", true);
		o.assignLabel("begin");
		o.setStringTableBase("begin");

		// add the compression header and set up variables next
		beginCompressedBlock(o);

		// begin the directory

		// dependency ordering
		// chunks will be written so that any chunk that they depend on, becomes
		// before them. Loops and cycles would be *bad*

		// to flatten the dependency list, each id is pushed on a stack
		tiny_stl::stack<uint32_t> dirIndices(allocator);
		for(auto const& node : chunkRegistry) dirIndices.push(node.first);

		// when id index gets popped, it stores itself in the order list
		// and pushs it all id of types it dependent on onto the stack.
		// This means that the order list has each index before the types its
		// dependent on.
		// later in the order list have fewer or nothing dependent on it
		// however this list contains duplicates, so need further processing
		// dependencies are on type but multiple chunks of the same type are allowed

		tiny_stl::vector<uint32_t> order(allocator);
		order.reserve(chunkRegistry.size());
		while(!dirIndices.empty()) {
			uint32_t id = dirIndices.top(); dirIndices.pop();
			auto const& entry = chunkRegistry.find(id)->second;
			order.push_back(id);
			for(uint32_t dep : entry.dependencies) {
				if(chunkRegistry.find(dep) != chunkRegistry.end()) {
					dirIndices.push(dep);
					continue;
				} else {
					debug_printf("Unknown dependency %u\n", dep);
					return false;
				}
			}
		}

		// we then go backwards through the ordering list, pushing the first
		// instance of each id onto another list. The produces the correct ordering
		// id of a type appear before anything that depends on it
		tiny_stl::vector<uint32_t> final(allocator);
		final.reserve(order.size());
		for(auto it = order.rbegin();it != order.rend();it++) {
			if(tiny_stl::find(final.begin(), final.end(), *it) == final.end()) {
				final.push_back(*it);
			}
		}

		o.assignLabel("directory");

		// each chunk gets a directory entry
		for (uint32_t id : final) {
			DirEntryWriter const& dw = chunkRegistry.find(id)->second;

			for(auto const & item : dw.items) {
				o.writeAs<uint32_t>(dw.id, "id type");
				o.writeAs<uint8_t>(dw.version, "Chunk version");
				o.align(4); // padd1-3
				o.addString(item.first);
				o.useLabel(item.first + "chunk", "", true);
				o.align(16); // padd4
				o.incrementVariable("DirectoryCount");
			}
		}


		// output chunks
		o.align();
		o.assignLabel("chunks", true);
		for(auto id : final) {
			auto const & entry = *chunkRegistry.find(id);

			o.align();
			entry.second.setup(nullptr, o);
			for( auto const & item : entry.second.items) {
				o.assignLabel(item.first + "chunk", false);
				entry.second.perItem(item.second, o);
			}
		}

		// output string table
		o.finishStringTable();

		o.assignLabel("fixups");
		size_t const numFixups = o.getFixupCount();
		o.setVariable("FixupCount", (int64_t) numFixups);
		for(size_t i = 0;i < numFixups;++i)
		{
			tiny_stl::string const& label = o.getFixup(i);
			o.useLabelAs<uint32_t>(label, "", false, false);
		}

		Binify_ContextHandle ctx = Binify_Create(o.c_str(), allocator);
		Utils::TrackingSlice<uint8_t> data {
			Utils::Slice<uint8_t >{ .data = (uint8_t *)(Binify_BinaryData(ctx)), .size = Binify_BinarySize(ctx) },
		};

		LZ4_FrameCompressionContext lz4ctx = LZ4_CreateFrameCompressor(result_, LZ4CS_64K, allocator);
		while(data.left() > 0 ) {
			size_t const amount = data.left() > LZ4CS_64K ? LZ4CS_64K : data.left();
			LZ4_FrameCompressNextChunk(lz4ctx, data.current, amount);
			data.increment(amount);
		}
		LZ4_FrameCompressFinishAndDestroy(lz4ctx);

		WriteHelper headerWriteHelper(allocator);
		writeBundleHeader(headerWriteHelper, data.slice.size, LZ4CS_64K);
		Binify_ContextHandle headerBinifyCtx = Binify_Create(headerWriteHelper.c_str(), allocator);
		Utils::Slice<uint8_t> headerData{ .data = (uint8_t *)(Binify_BinaryData(headerBinifyCtx)), .size = Binify_BinarySize(headerBinifyCtx) };
		VFile_Seek(result_, 0, VFile_SD_Begin);
		VFile_Write(result_, headerData.data, headerData.size);

		Binify_Destroy(headerBinifyCtx);
		Binify_Destroy(ctx);
		return true;
	}

	void BundleWriter::writeBundleHeader(WriteHelper& h, size_t const uncompressedSize, size_t const decompressionBufferSize) const
	{
		// write header
		h.comment("---------------------------------------------");
		h.comment("Bundle Header");
		h.comment("---------------------------------------------");
		h.setAddressLength(addressLength);
		h.setDefaultType<uint32_t>();

		h.addEnum("HeaderFlags");
	  h.addEnumValue("HeaderFlags", "32Bit", RBHF_Is32Bit);
		h.addEnumValue("HeaderFlags", "64BitFixups", RBHF_64BitFixups);

		// magic
		static_assert("BUND"_bundle_id == RESOURCE_BUNDLE_ID('B','U','N','D'));
		h.write("BUND"_bundle_id, "magic");

		uint32_t flags = (h.getAddressLength() == 32) ? RBHF_Is32Bit : RBHF_64BitFixups;

		h.writeFlagsAs<uint16_t>("HeaderFlags", flags, "HeaderFlags");
		h.writeAs<uint8_t>(ResourceBundle_MajorVersion, ResourceBundle_MinorVersion, "major, minor version");
		h.writeAs<uint32_t>(uncompressedSize + sizeof(ResourceBundle_Header), "Total uncompressed size including header");
		h.writeAs<uint32_t>( decompressionBufferSize,"Size needed for decompression buffer");
	}

	void BundleWriter::beginCompressedBlock(Binify::WriteHelper& h) {
		h.comment("---------------------------------------------");
		h.comment("Compressed Block");
		h.comment("---------------------------------------------");
		h.setAddressLength(addressLength);
		h.setDefaultType<uint32_t>();
		o.setVariable("DirectoryCount", 0, true);
		o.setVariable("FixupCount", 0, true);
		h.writeVariableAs<uint32_t>("DirectoryCount");
		h.writeVariableAs<uint32_t>("FixupCount");
		h.useLabelAs<uint32_t>("directory","", true, false, "directory offset");
		h.useLabelAs<uint32_t>("fixups","", true, false, "Fixups offset");
		h.align(16);
	}

	tiny_stl::string BundleWriter::outputText() {
		return {o.c_str(), allocator};
	}

} // end namespace