#include "core/core.h"
#include "tiny_stl/stack.hpp"
#include "tiny_stl/algorithm.hpp"
#include "data_binify/data_binify.h"
#include "data_utils/lz4.h"
#include "data_utils/crc32c.h"
#include "resource_bundle.h"
#include "resource_bundle_writer.hpp"
#include "data_binify/write_helper.hpp"
#include "dbg/print.h"
namespace Binny {
	using namespace Binify;

	BundleWriter::BundleWriter(int addressLength_, Memory_Allocator* allocator_, Memory_Allocator* tempAllocator_) :
			addressLength(addressLength_),
			allocator(allocator_),
			tempAllocator(tempAllocator_),
			dirEntries(allocator_),
			o(allocator_)
	{
		assert(addressLength == 64 || addressLength == 32);
		o.setAddressLength(addressLength);
	}


	bool BundleWriter::addRawTextChunk(tiny_stl::string const& name_,
	                                   uint32_t id_,
	                                   uint8_t version_,
	                                   uint32_t directoryFlags_,
	                                   tiny_stl::vector<uint32_t> const& dependencies_,
	                                   tiny_stl::string const& text_)
	{
		bool nullTerminated = text_.back() == '\0';

		tiny_stl::vector<uint8_t> bin(text_.size() + sizeof(ResourceBundle_ChunkHeader32) + (nullTerminated ? 0 : 1), allocator);
		memcpy(bin.data() + sizeof(ResourceBundle_ChunkHeader32), text_.data(), text_.size());
		if (!nullTerminated)
		{
			bin[sizeof(ResourceBundle_ChunkHeader32) + text_.size()] = 0; // null terminate string
		}

		// write chunk header
		auto* cheader = (ResourceBundle_ChunkHeader32*) bin.data();
		cheader->dataSize = text_.size() + 1;
		cheader->fixupOffset = 0;
		cheader->fixupSize = 0;
		cheader->dataOffset = sizeof(ResourceBundle_ChunkHeader32);
		cheader->version = version_;

		return addChunkInternal(name_,
														id_,
														directoryFlags_,
														dependencies_,
														Utils::Slice<uint8_t const> { .data = bin.data(), .size = bin.size() });
	}

	bool BundleWriter::addRawBinaryChunk(tiny_stl::string const& name_,
	                                     uint32_t id_,
	                                     uint8_t version_,
	                                     uint32_t directoryFlags_,
	                                     tiny_stl::vector<uint32_t> const& dependencies_,
	                                     Utils::Slice<uint8_t const> const bin_)
	{
		tiny_stl::vector<uint8_t> bin(bin_.size + sizeof(ResourceBundle_ChunkHeader32), allocator);
		memcpy(bin.data() + sizeof(ResourceBundle_ChunkHeader32), bin_.data, bin_.size);

		// write chunk header
		auto cheader = (ResourceBundle_ChunkHeader32*) bin.data();
		cheader->dataSize = bin_.size;
		cheader->fixupOffset = 0;
		cheader->fixupSize = 0;
		cheader->dataOffset = sizeof(ResourceBundle_ChunkHeader32);
		cheader->version = version_;

		return addChunkInternal(tiny_stl::string(name_, allocator),
														id_,
														directoryFlags_,
														dependencies_,
														Utils::Slice<uint8_t const> { .data = bin.data(), .size = bin.size() });
	}

	bool BundleWriter::addChunkInternal(
			tiny_stl::string const& name_,
			uint32_t id_,
			uint32_t flags_,
			tiny_stl::vector<uint32_t> const & dependencies_,
			Utils::Slice<uint8_t const> const bin_)
	{
		if(addressLength == 32 && bin_.size >= (1ull << 32)) return false;

		uint32_t const uncompressedCrc32c = CRC32C_Calculate(0, bin_.data, bin_.size);
		size_t const maxSize = LZ4_CompressionBoundFromInputSize(bin_.size);

		auto compressedData = new tiny_stl::vector<uint8_t>(maxSize, allocator);
		size_t const compressedSize = LZ4_CompressHigh(bin_.data,bin_.size,
																compressedData->data(),compressedData->size(),
		                            LZ4_MAX_COMPRESSION_LEVEL);

		bool compressedOkay = compressedSize > 0;
		if (addressLength == 32 && compressedSize >= (1ull << 32)) compressedOkay = false;
		if(!compressedOkay) {
			delete compressedData;
			return false;
		}
		compressedData->resize(compressedSize);

		// if compression made this block bigger, use the uncompressed data and mark it by
		// having uncompressed size == compressed size in the file
	//			if (compressedData->size() >= bin_.size()) *compressedData = bin_;


		DirEntryWriter entry = {
				id_,
				flags_,
				name_,
				compressedData->size(),
				bin_.size,
				CRC32C_Calculate(0, compressedData->data(), compressedData->size()),
				uncompressedCrc32c,
				compressedData,
				dependencies_
		};

		dirEntries.push_back(entry);
		o.reserveLabel(name_ + "chunk");
		return true;
	}

	bool BundleWriter::addChunk(tiny_stl::string const& name_,
	                            uint32_t id_,
	                            uint8_t version_,
	                            uint32_t directoryFlags_,
	                            tiny_stl::vector<uint32_t> const& dependencies_,
	                            ChunkWriter writer_)
	{
		WriteHelper h(allocator);

		// add chunk header
		writeChunkHeader(version_);

		h.writeLabel("data");
		writer_(h);
		h.finishStringTable();
		h.align();
		h.writeLabel("dataEnd");
		h.writeLabel("fixups");
		size_t const numFixups = h.getFixupCount();
		for(size_t i = 0;i < numFixups;++i)
		{
			tiny_stl::string const& label = h.getFixup(i);
			h.useLabel(label, "", false, false);
		}
		h.writeLabel("fixupsEnd");

		char const* binifyText = h.c_str();

		/*
		Binify_Create(binifyText, )

		std::string log;
		std::vector<uint8_t> data;
		std::stringstream binData;
		bool okay = BINIFY_StringToOStream(binifyText, &binData, log);
		if(!okay) return false;
		std::string tmp = binData.str();
		data.resize(tmp.size());
		std::memcpy(data.data(), tmp.data(), tmp.size());

		if (!log.empty() || logBinifyText)
		{
			LOG_S(INFO) << name_ << "_Chunk:" << log;
			LOG_S(INFO) << std::endl << binifyText;
		}
	*/
		return false; // addChunkInternal(name_, id_, flags_, dependencies_, data);
	}

	bool BundleWriter::build(tiny_stl::vector<uint8_t>& result_)
	{
		writeBundleHeader();

		// begin the directory
		o.reserveLabel("begin", true);
		o.writeLabel("begin");

		o.addEnum("ChunkFlags");

		// dependency ordering
		// chunks will be written so that any chunk that are depended on, become
		// before teh chunks that depend on them. Loops and cycles would be *bad*


		// to flatten the dependency list, each index is pushed on a stack
		tiny_stl::stack<uint32_t> dirIndices(allocator);
		for(auto i = 0u; i < dirEntries.size(); ++i)
		{
			dirIndices.push(i);
		}

		// when each index gets popped, it stores itself in the order list
		// and pushs it all indices of types it dependent on onto the stack.
		// This means that the order list has each index before the types its
		// dependent on.
		// later in the order list have fewer or nothing dependent on it
		// however this list contains duplicates, so need further processing
		// dependencies are on type but multiple chunks of the same type are allowed

		tiny_stl::vector<uint32_t> order(allocator);
		order.reserve(dirEntries.size());
		while(!dirIndices.empty())
		{
			uint32_t index = dirIndices.top(); dirIndices.pop();
			auto const& entry = dirEntries[index];
			order.push_back(index);
			for(uint32_t dep : entry.dependencies)
			{
				auto fit = dirEntries.cbegin();
				do
				{
					for(auto& it = fit;dirEntries.cend(); ++it) {
						if(it->id == dep) {
							dirIndices.push(tiny_stl::distance(dirEntries.cbegin(), it));
							++fit;
						}
					}
				} while(fit != dirEntries.cend());
			}
		}
		// we then go backwards through the ordering list, pushing the first
		// instance of each index onto another list. The produces the correct ordering
		// indices of a type appear before anything that depends on it
		tiny_stl::vector<uint32_t> final(allocator);
		final.reserve(order.size());
		for(auto it = order.rbegin();it != order.rend();it++)
		{
			if(std::find(final.begin(), final.end(), *it) == final.end())
			{
				final.push_back(*it);
			}
		}

		tiny_stl::string lastName {"chunks", allocator };
		for (uint32_t index : final)
		{
			DirEntryWriter const& dw = dirEntries[index];

			o.write(dw.id, "id type");
			o.writeAs<uint32_t>(dw.compressedCrc32c, "stored crc32c");
			o.writeAs<uint32_t>(dw.uncompressedCrc32c, "unpacked crc32c");
			o.writeFlags("ChunkFlags", dw.flags, "Chunk flags");

			o.addString(dw.name);
			o.useLabel(dw.name + "chunk", lastName, false, false );

			o.writeSize(dw.compressedSize, "stored size");
			o.writeSize(dw.uncompressedSize, "unpacked size");

			o.incrementVariable("DirEntryCount");
			lastName = dw.name + "chunk";
		}
		o.writeLabel("beginEnd");

		// output string table
		o.finishStringTable();

		// output pointer fixups

		// output chunks
		o.align();
		o.writeLabel("chunks");
		for (auto& entry : dirEntries)
		{
			assert(entry.chunk != nullptr);
			o.align();
			o.writeLabel(entry.name + "chunk");
			o.writeByteArray(Utils::Slice<uint8_t const>{ entry.chunk->begin(), entry.chunk->end() });
			delete entry.chunk;
			entry.chunk = nullptr;
		}
		o.writeLabel("chunksEnd");

		/*
		// convert
		std::string binifyText = o.ostr.str();
		std::string log;
		std::stringstream binData;
		bool okay = BINIFY_StringToOStream(binifyText, &binData, log);
		if(!okay) return false;
		std::string tmp = binData.str();
		result_.resize(tmp.size());
		std::memcpy(result_.data(), tmp.data(), tmp.size());
*/
		if (logBinifyText)
		{
			debug_print(o.c_str());
	//				LOG_S(INFO) << "Bundle: ";
	//				LOG_S(INFO) << std::endl << binifyText;
		}

		return true;
	}

	void BundleWriter::writeChunkHeader(uint8_t version_, bool use64BitFixups)
	{
		// write header
		o.comment("---------------------------------------------");
		o.comment("Chunk");
		o.comment("---------------------------------------------");
		o.writeAddressType();

		o.writeLabel("chunk_begin", true);
		o.setStringTableBase("chunk_begin");
		o.reserveLabel("fixups");
		o.reserveLabel("data", true);
		o.reserveLabel("fixupsEnd");
		o.reserveLabel("dataEnd");

		o.useLabel("data", "chunk_begin", false, false );
		o.sizeOfBlock("data");
		o.useLabel("fixups", "chunk_begin", false, false );
		o.sizeOfBlockAs<uint16_t>("fixups");
		o.writeAs<uint8_t>(version_, "Version");
		uint32_t const flags = (o.getAddressLength() == 32) ? RBHF_Is32Bit : 0;
		o.writeFlagsAs<uint8_t>("HeaderFlag", flags);

		o.align();
		o.setDefaultType<uint32_t>();
		o.setStringTableBase("data");
	}

	void BundleWriter::writeBundleHeader()
	{
		// write header
		o.comment("---------------------------------------------");
		o.comment("Bundle");
		o.comment("---------------------------------------------");
		o.setDefaultType<uint32_t>();

		o.addEnum("HeaderFlags");
	  o.addEnumValue("HeaderFlags", "32Bit", RBHF_Is32Bit);
		o.addEnum("ChunkHeaderFlags");
		o.addEnumValue("ChunkHeaderFlags", "64BitFixups", RBCF_64BitFixups);
		o.setVariable("DirectoryCount", 0, true);
		o.setVariable("UncompressedSize", 0, true);

		// magic
		o.write("BUND"_bundle_id, "magic");
		static_assert("BUND"_bundle_id == RESOURCE_BUNDLE_ID('B','U','N','D'));

		uint32_t flags = (o.getAddressLength() == 32) ? RBHF_Is32Bit : 0;
		o.writeFlags("HeaderFlags", flags);
		o.writeAs<uint8_t>(ResourceBundle_MajorVersion, ResourceBundle_MinorVersion, "major, minor version");
		o.writeExpressionAs<uint32_t>(o.getVariable("DirectoryCount"), "Directory count");
		o.sizeOfBlock("stringTable");
		o.writeExpressionAs<uint32_t>(o.getVariable("UncompressedSize"), "Total uncompressed size");
		o.align(32);
		o.incrementVariable("UncompressedSize", 32, "include header size");
	}

} // end namespace