#pragma once

#include "core/core.h"
#include "memory/memory.h"
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include "memory/memory.h"
#include "data_binify/write_helper.hpp"

namespace Binny {
		template <class T>
		class stdAllocatorAdaptor
		{
		public:
				using value_type    = T;
				stdAllocatorAdaptor(Memory_Allocator* allocator_) : allocator(allocator_) {}
				template <class U> stdAllocatorAdaptor(stdAllocatorAdaptor<U> const&) noexcept {}

				value_type* allocate(std::size_t n)
				{
					return (value_type*) MCALLOC(allocator,n, sizeof(value_type));
				}

				void deallocate(value_type* p, std::size_t) noexcept  // Use pointer if pointer is not a value_type*
				{
					MFREE(allocator, p);
				}

		private:
				Memory_Allocator* allocator;
		};

		template <class T, class U>
		bool operator==(stdAllocatorAdaptor<T> const&, stdAllocatorAdaptor<U> const&) noexcept
		{
			return true;
		}

		template <class T, class U>
		bool operator!=(stdAllocatorAdaptor<T> const& x, stdAllocatorAdaptor<U> const& y) noexcept
		{
			return !(x == y);
		}

		class BundleWriter
		{
		public:

				BundleWriter( int addressLength_, Memory_Allocator* allocator, Memory_Allocator* tempAllocator );

				using ChunkWriter = std::function<void( Binify::WriteHelper& helper )>;

				/// add a raw text chunk, will be stored independently in its own chunk
				/// @return true if successful
				bool addRawTextChunk( std::string const& name_,
				                      uint32_t id_,
				                      uint16_t majorVersion_,
				                      uint16_t minorVersion_,
				                      uint32_t flags_,
				                      std::vector<uint32_t> const& dependencies_,
				                      std::string const& text_ );

				/// add a raw binary chunk, will be stored in its own chunk without any processing
				/// @return true if successful
				bool addRawBinaryChunk( std::string const& name_,
				                        uint32_t id_,
				                        uint16_t majorVersion_,
				                        uint16_t minorVersion_,
				                        uint32_t flags_,
				                        std::vector<uint32_t> const& dependencies_,
				                        std::vector<uint8_t> const& bin_ );

				/// @param ChunkWriter will be called at build time
				/// @return true if successful
				bool addChunk( std::string const& name_,
				               uint32_t id_,
				               uint16_t majorVersion_,
				               uint16_t minorVersion_,
				               uint32_t flags_,
				               std::vector<uint32_t> const& dependencies_,
				               ChunkWriter writer_ );

				/// @param userData_ a 64 bit in that store in the header, usually a cache / re-gen marker
				/// @param result_ where the bundle data will be put
				/// @return true if successful
				bool build( uint64_t const userData_, std::vector<uint8_t>& result_ );

				void setLogBinifyText()
				{ logBinifyText = true; }

		private:
				void writeChunkHeader(uint16_t majorVersion_, uint16_t minorVersion_);
				void writeBundleHeader(uint64_t const userData_);

				bool addChunkInternal( std::string const& name_,
				                       uint32_t id_,
				                       uint32_t flags_,
				                       std::vector<uint32_t> const& dependencies_,
				                       std::vector<uint8_t> const& bin_);

				int addressLength;
				std::unique_ptr<Binify::WriteHelper> helper;
				Binify::WriteHelper& o;

				struct DirEntryWriter
				{
						uint32_t id;
						uint32_t flags;
						std::string name;
						size_t compressedSize;
						size_t uncompressedSize;
						uint32_t compressedCrc32c;
						uint32_t uncompressedCrc32c;
						std::vector<uint8_t> *chunk;
						std::vector<uint32_t> dependencies;
				};
				stdAllocatorAdaptor<DirEntryWriter> allocatorAdaptor;

				std::vector<DirEntryWriter, stdAllocatorAdaptor<DirEntryWriter>> dirEntries;
				bool logBinifyText = false;
		};

} // end namespace
