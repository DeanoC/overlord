#include "core/core.h"
#include <string>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include "data_binify/data_binify.h"
#include "data_utils/lz4.h"
#include "data_utils/crc32c.h"
#include "resource_bundle.h"
#include "resource_bundle_writer.hpp"
#include "data_binify/write_helper.hpp"


namespace Binny {
		using namespace Binify;
		using namespace std::string_literals;
		using namespace std::string_view_literals;

		BundleWriter::BundleWriter(int addressLength_, Memory_Allocator* allocator, Memory_Allocator* tempAllocator) :
				addressLength(addressLength_),
				helper(std::make_unique<WriteHelper>()),
				allocatorAdaptor(allocator),
				dirEntries(allocatorAdaptor),
				o(*helper)
		{
			assert(addressLength == 64 || addressLength == 32);
			o.setAddressLength(addressLength);
		}


		bool BundleWriter::addRawTextChunk(std::string const& name_,
		                                   uint32_t id_,
		                                   uint16_t majorVersion_,
		                                   uint16_t minorVersion_,
		                                   uint32_t flags_,
		                                   std::vector<uint32_t> const& dependencies_,
		                                   std::string const& text_)
		{
			bool nullTerminated = text_.back() == '\0';

			std::vector<uint8_t> bin(text_.size() + sizeof(ResourceBundle_ChunkHeader32) + (nullTerminated ? 0 : 1));
			std::memcpy(bin.data() + sizeof(ResourceBundle_ChunkHeader32), text_.data(), text_.size());
			if (nullTerminated == false)
			{
				bin[sizeof(ResourceBundle_ChunkHeader32) + text_.size()] = 0; // null terminate string
			}

			// write chunk header
			ResourceBundle_ChunkHeader32* cheader = (ResourceBundle_ChunkHeader32*) bin.data();
			cheader->dataSize = text_.size() + 1;
			cheader->fixupOffset = 0;
			cheader->fixupCount = 0;
			cheader->dataOffset = sizeof(ResourceBundle_ChunkHeader32);
			cheader->majorVersion = majorVersion_;
			cheader->minorVersion = minorVersion_;

			return addChunkInternal(name_, id_, flags_, dependencies_, bin);
		}

		bool BundleWriter::addRawBinaryChunk(std::string const& name_,
		                                     uint32_t id_,
		                                     uint16_t majorVersion_,
		                                     uint16_t minorVersion_,
		                                     uint32_t flags_,
		                                     std::vector<uint32_t> const& dependencies_,
		                                     std::vector<uint8_t> const& bin_)
		{
			std::vector<uint8_t> bin(bin_.size() + sizeof(ResourceBundle_ChunkHeader32));
			std::memcpy(bin.data() + sizeof(ResourceBundle_ChunkHeader32), bin_.data(), bin_.size());

			// write chunk header
			ResourceBundle_ChunkHeader32* cheader = (ResourceBundle_ChunkHeader32*) bin.data();
			cheader->dataSize = bin_.size();
			cheader->fixupOffset = 0;
			cheader->fixupCount = 0;
			cheader->dataOffset = sizeof(ResourceBundle_ChunkHeader32);
			cheader->majorVersion = majorVersion_;
			cheader->minorVersion = minorVersion_;

			return addChunkInternal(std::string(name_), id_, flags_, dependencies_, bin);
		}

		bool BundleWriter::addChunkInternal(
				std::string const& name_,
				uint32_t id_,
				uint32_t flags_,
				std::vector<uint32_t> const& dependencies_,
				std::vector<uint8_t> const& bin_)
		{
			uint32_t const uncompressedCrc32c = CRC32C_Calculate(0, bin_.data(), bin_.size());
			int const maxSize = LZ4_CompressionBoundFromInputSize((int)bin_.size());

			std::vector<uint8_t>*  compressedData = new std::vector<uint8_t>(maxSize);
			{
				int okay = LZ4_CompressHigh(bin_.data(),bin_.size(),
																		compressedData->data(),compressedData->size(),
				                            LZ4_MAX_COMPRESSION_LEVEL);
				if(!okay) return false;
				compressedData->resize(okay);
			}

			// if compression made this block bigger, use the uncompressed data and mark it by
			// having uncompressed size == 0 in the file
			if (compressedData->size() >= bin_.size())
			{
				*compressedData = bin_;
			}

			if (addressLength == 32)
			{
				if(compressedData->size() >= (1ull << 32)) return false;
				if(bin_.size() >= (1ull << 32)) return false;
			}

			DirEntryWriter entry =
					{
							id_,
							flags_,
							name_,
							compressedData->size(),
							bin_.size(),
							CRC32C_Calculate(0, compressedData->data(), compressedData->size()),
							uncompressedCrc32c,
							compressedData,
							dependencies_
					};

			dirEntries.push_back(entry);
			o.reserveLabel(name_ + "chunk"s);
			return true;
		}

		bool BundleWriter::addChunk(std::string const& name_,
		                            uint32_t id_,
		                            uint16_t majorVersion_,
		                            uint16_t minorVersion_,
		                            uint32_t flags_,
		                            std::vector<uint32_t> const& dependencies_,
		                            ChunkWriter writer_)
		{
			WriteHelper h;

			// add chunk header
			writeChunkHeader(majorVersion_, minorVersion_);

			h.writeLabel("data"s);
			writer_(h);
			h.finishStringTable();
			h.align();
			h.writeLabel("dataEnd"s);
			h.writeLabel("fixups"s);
			size_t const numFixups = h.getFixupCount();
			for(size_t i = 0;i < numFixups;++i)
			{
				std::string const& label = h.getFixup(i);
				h.useLabel(label, "", false, false);
			}
			h.writeLabel("fixupsEnd"s);

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

		bool BundleWriter::build(uint64_t const userData_, std::vector<uint8_t>& result_)
		{
			using namespace std::string_literals;

			writeBundleHeader(userData_);

			// begin the directory
			o.reserveLabel("begin"s, true);
			o.writeLabel("begin"s);

			o.addEnum("ChunkFlag");
//			o.addEnumValue("ChunkFlag", "TempAlloc"s, Bundle::ChunkFlag_TempAlloc);

			// dependency ordering
			// chunks will be written so that any chunk that are depended on, become
			// before teh chunks that depend on them. Loops and cycles would be *bad*


			// to flatten the dependency list, each index is pushed on a stack
			std::stack<uint32_t> dirIndices;
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

			std::vector<uint32_t> order;
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
						fit = std::find_if(fit, dirEntries.cend(),
						                   [dep](auto const& e_){
								                   return e_.id == dep;
						                   } );
						if(fit != dirEntries.end())
						{
							dirIndices.push(std::distance(dirEntries.cbegin(), fit));
							fit++;
						}
					} while(fit != dirEntries.cend());
				}
			}
			// we then go backwards through the ordering list, pushing the first
			// instance of each index onto another list. The produces the correct ordering
			// indices of a type appear before anything that depends on it
			std::vector<uint32_t> final;
			final.reserve(order.size());
			for(auto it = order.rbegin();it != order.rend();it++)
			{
				if(std::find(final.begin(), final.end(), *it) == final.end())
				{
					final.push_back(*it);
				}
			}

			std::string lastName = "chunks"s;
			for (uint32_t index : final)
			{
				DirEntryWriter const& dw = dirEntries[index];

				o.write(dw.id, "id type"s);
				o.writeAs<uint32_t>(dw.compressedCrc32c, "stored crc32c"s);
				o.writeAs<uint32_t>(dw.uncompressedCrc32c, "unpacked crc32c"s);
				o.writeFlags("ChunkFlag", dw.flags, "Chunk flags");

				o.addString(dw.name);
				o.useLabel(dw.name + "chunk"s, lastName, false, false );

				o.writeSize(dw.compressedSize, "stored size"s);
				o.writeSize(dw.uncompressedSize, "unpacked size"s);

				o.incrementVariable("DirEntryCount"s);
				lastName = dw.name + "chunk"s;
			}
			o.writeLabel("beginEnd"s);

			// output string table
			o.finishStringTable();

			// output pointer fixups

			// output chunks
			o.align();
			o.writeLabel("chunks"s);
			for (auto& entry : dirEntries)
			{
				assert(entry.chunk != nullptr);
				o.align();
				o.writeLabel(entry.name + "chunk"s);
				o.writeByteArray(*entry.chunk);
				delete entry.chunk;
				entry.chunk = nullptr;
			}
			o.writeLabel("chunksEnd"s);

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

			if (!log.empty() || logBinifyText)
			{
//				LOG_S(INFO) << "Bundle: ";
//				LOG_S(INFO) << std::endl << binifyText;
			}
			 */
			return true;
		}

		void BundleWriter::writeChunkHeader(uint16_t majorVersion_, uint16_t minorVersion_)
		{
			// write header
			o.comment("---------------------------------------------"s);
			o.comment("Chunk"s);
			o.comment("---------------------------------------------"s);
			o.writeAddressType();

			o.writeLabel("chunk_begin"s, true);
			o.setStringTableBase("chunk_begin"s);
			o.reserveLabel("fixups"s);
			o.reserveLabel("data"s, true);
			o.reserveLabel("fixupsEnd"s);
			o.reserveLabel("dataEnd"s);

			o.sizeOfBlock("fixups"s);
			o.sizeOfBlock("data"s);
			o.useLabel("fixups"s, "chunk_begin"s, false, false );
			o.useLabel("data"s, "chunk_begin"s, false, false );
			o.writeAs<uint16_t>(majorVersion_, "Major Version"s);
			o.writeAs<uint16_t>(minorVersion_, "Minor Version"s);
			o.align();
			o.setDefaultType<uint32_t>();
			o.setStringTableBase("data"s);
		}

		void BundleWriter::writeBundleHeader(uint64_t const userData_)
		{
			// write header
			o.comment("---------------------------------------------"s);
			o.comment("Bundle"s);
			o.comment("---------------------------------------------"s);
			o.setDefaultType<uint32_t>();

			o.addEnum("HeaderFlag");
//			o.addEnumValue("HeaderFlag", "64Bit"s, Bundle::HeaderFlag_64Bit);
//			o.addEnumValue("HeaderFlag", "32Bit"s, Bundle::HeaderFlag_32Bit);
			o.setVariable("DirEntryCount"s, 0, true);

			// magic
//			o.write("BUND"_bundle_id, "magic"s);
/*
			// flags
			uint32_t flags = (o.getAddressLength() == 32) ? Bundle::HeaderFlag_32Bit : Bundle::HeaderFlag_64Bit;
			write_flags("HeaderFlag"s, flags);

			// version
			write_as<uint16_t>(Bundle::majorVersion, Bundle::minorVersion, "major, minor version"s);

			// micro offsets
			write_expression_as<uint16_t>("stringTable - beginEnd"s, "strings micro offset"s);
			write_expression_as<uint16_t>("chunks - stringTableEnd"s, "chunks micro offset"s);

			// 64 bit user data
			write_as<uint64_t>(userData_);

			// string table size
			size_of_block("stringTable"s);

			// directory entry count
			write_expression_as<uint32_t>(get_variable("DirEntryCount"), "Directory entry count"s);
*/
		}

} // end namespace