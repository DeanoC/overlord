#pragma once

#include "core/core.h"
#include "core/utils.hpp"
#include "memory/memory.hpp"
#include "tiny_stl/string.hpp"
#include "memory/memory.h"
#include "data_binify/write_helper.hpp"
#include "resource_bundle/resource_bundle.h"
#include <functional>

namespace Binny {

		class BundleWriter
		{
		public:

				BundleWriter( int addressLength_, Memory_Allocator* allocator_);

				using ChunkWriter = std::function<void( void * userData, Binify::WriteHelper& helper )>;

				/// @param ChunkWriter will be called at build time for each item of this chunk
				/// @return true if successful
				bool registerChunk( uint32_t id_,
				                    uint8_t version_,
				                    tiny_stl::vector<uint32_t> const& dependencies_,
														ChunkWriter const & setup_,
				                    ChunkWriter const & item_ );

				void addItemToChunk(uint32_t id_, tiny_stl::string const & name_, void* item_);

				/// @param result_ where the bundle data will be put
				/// @return true if successful
				bool build(VFile_Handle result_ );

				tiny_stl::string outputText();

		private:
				void writeBundleHeader(Binify::WriteHelper& h, size_t uncompressedSize, size_t decompressionBufferSize) const;
				void beginCompressedBlock(Binify::WriteHelper& h);

				int addressLength;
				Binify::WriteHelper o;
				Memory_Allocator* allocator;

				typedef tiny_stl::pair<tiny_stl::string, void*> DirNamePair;

				struct DirEntryWriter
				{
						uint32_t id;
						uint8_t version;
						tiny_stl::vector<uint32_t> dependencies;
						ChunkWriter setup;
						ChunkWriter perItem;
						tiny_stl::vector<DirNamePair> items;
				};

				tiny_stl::unordered_map<uint32_t, DirEntryWriter> chunkRegistry;
				bool logBinifyText = false;
		};

} // end namespace
