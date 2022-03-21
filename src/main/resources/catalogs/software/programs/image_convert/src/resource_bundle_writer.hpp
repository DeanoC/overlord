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

				BundleWriter( int addressLength_, bool fixup64bit_, Memory_Allocator* allocator_);

				using ChunkWriter = std::function<void( void * userData, Binify::WriteHelper& helper )>;

				void setCompressionBlockSize(uint32_t size_) { compressionBlockSize = size_; }

				/// @param ChunkWriter will be called at build time for each item of this chunk
				/// @return true if successful
				bool registerChunk( tiny_stl::string const& name_,
				               uint32_t id_,
				               uint8_t version_,
				               tiny_stl::vector<uint32_t> const& dependencies_,
				               ChunkWriter const& writer_ );

				void addItemToChunk(uint32_t id_, void* item);

				/// @param result_ where the bundle data will be put
				/// @return true if successful
				bool build(VFile_Handle result_ );

				void setLogBinifyText() { logBinifyText = true; }

		private:
				void writeBundleHeader(Binify::WriteHelper& h, size_t uncompressedSize, size_t decompressionBufferSize) const;
				void beginCompressedBlock(Binify::WriteHelper& h);

				bool addChunkInternal( tiny_stl::string const& name_,
				                       uint32_t id_,
				                       uint32_t flags_,
				                       tiny_stl::vector<uint32_t> const& dependencies_,
															 Utils::Slice<uint8_t const > bin_);

				int addressLength;
				Binify::WriteHelper o;
				Memory_Allocator* allocator;

				struct DirEntryWriter
				{
						uint32_t id;
						uint8_t version;
						tiny_stl::string name;
						tiny_stl::vector<uint32_t> dependencies;
						ChunkWriter writer;
						tiny_stl::vector<void*> items;
				};

				tiny_stl::unordered_map<uint32_t, DirEntryWriter> chunkRegistry;
				bool logBinifyText = false;
				uint32_t compressionBlockSize;
				bool fixup64Bits;
		};

} // end namespace
