#pragma once

#include "core/core.h"
#include "core/utils.hpp"
#include "utils/slice.hpp"
#include "memory/memory.hpp"
#include "tiny_stl/string.hpp"
#include "memory/memory.h"
#include "data_binify/write_helper.hpp"
#include "resource_bundle.h"

namespace Binny {

		class BundleWriter
		{
		public:

				BundleWriter( int addressLength_, Memory_Allocator* allocator_, Memory_Allocator* tempAllocator_ );

				using ChunkWriter = std::function<void( Binify::WriteHelper& helper )>;

				/// add a raw text chunk, will be stored independently in its own chunk
				/// @return true if successful
				bool addRawTextChunk( tiny_stl::string const& name_,
				                      uint32_t id_,
				                      uint8_t version_,
				                      uint32_t directoryFlags_,
				                      tiny_stl::vector<uint32_t> const& dependencies_,
				                      tiny_stl::string const& text_ );

				/// add a raw binary chunk, will be stored in its own chunk without any processing
				/// @return true if successful
				bool addRawBinaryChunk( tiny_stl::string const& name_,
				                        uint32_t id_,
				                        uint8_t version_,
				                        uint32_t directoryFlags_,
				                        tiny_stl::vector<uint32_t> const& dependencies_,
				                        Utils::Slice<uint8_t const> bin_ );

				/// @param ChunkWriter will be called at build time
				/// @return true if successful
				bool addChunk( tiny_stl::string const& name_,
				               uint32_t id_,
				               uint8_t version_,
				               uint32_t directoryFlags_,
				               tiny_stl::vector<uint32_t> const& dependencies_,
				               ChunkWriter writer_ );

				/// @param result_ where the bundle data will be put
				/// @return true if successful
				bool build(tiny_stl::vector<uint8_t>& result_ );

				void setLogBinifyText() { logBinifyText = true; }

		private:
				void writeChunkHeader(uint8_t version_, bool use64BitFixups = true);
				void writeBundleHeader();

				bool addChunkInternal( tiny_stl::string const& name_,
				                       uint32_t id_,
				                       uint32_t flags_,
				                       tiny_stl::vector<uint32_t> const& dependencies_,
															 Utils::Slice<uint8_t const > bin_);

				int addressLength;
				Binify::WriteHelper o;
				Memory_Allocator* allocator;
				Memory_Allocator* tempAllocator;

				struct DirEntryWriter
				{
						uint32_t id;
						uint32_t flags;
						tiny_stl::string name;
						size_t compressedSize;
						size_t uncompressedSize;
						uint32_t compressedCrc32c;
						uint32_t uncompressedCrc32c;
						tiny_stl::vector<uint8_t> *chunk;
						tiny_stl::vector<uint32_t> dependencies;
				};

				tiny_stl::vector<DirEntryWriter> dirEntries;
				bool logBinifyText = true;
		};

} // end namespace
