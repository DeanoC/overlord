// License Summary: MIT see LICENSE file
#include "core/core.h"
#include "memory/memory.h"

#include "dbg/assert.h"
#include "dbg/print.h"
#include "dbg/ansi_escapes.h"
#include "multi_core/core_local.h"
#include "core/utf8.h"
#include "memory/memory.h"

static uint64_t g_allocCounter = 0;
uint64_t Memory_TrackerBreakOnAllocNumber = 0; // set this here or in code before the allocation occurs to break

//#define MEMORY_TRACKING 0 // will switch off the cost of tracking bar 3 pushing and memory for the strings
// #define MEMORY_TRACKING_SETUP 0 in header will remove this overhead as well..

#if !defined(MEMORY_TRACKING)
#define MEMORY_TRACKING 1
#endif

#if !defined(MEMORY_TRACKING) && (defined(MEMORY_TRACKING_SETUP) && MEMORY_TRACKING_SETUP != 0)
#define MEMORY_TRACKING 1
#endif

#if MEMORY_TRACKING == 1 && (!defined(MEMORY_TRACKING_SETUP) && MEMORY_TRACKING_SETUP == 0)
#error MEMORY_TRACKING requires MEMORY_TRACKING_SETUP == 1
#endif

#if MEMORY_TRACKING_SETUP == 1

#if CPU_host
#if IKUY_HOST_PLATFORM_OS == IKUY_HOST_OS_OSX || IKUY_HOST_PLATFORM == IKUY_HOST_PLATFORM_UNIX
#include <unistd.h>
#include <pthread.h>

static pthread_mutex_t g_allocMutex = PTHREAD_MUTEX_INITIALIZER;
#define MUTEX_CREATE
#define MUTEX_DESTROY
#define MUTEX_LOCK   pthread_mutex_lock(&g_allocMutex);
#define MUTEX_UNLOCK pthread_mutex_unlock(&g_allocMutex);

#elif IKUY_HOST_PLATFORM == IKUY_HOST_PLATFORM_WINDOWS
#include "al2o3_platform/windows.h"

static CRITICAL_SECTION g_allocMutex;
#define MUTEX_CREATE InitializeCriticalSection(&g_allocMutex);
#define MUTEX_DESTROY DeleteCriticalSection(&g_allocMutex);
#define MUTEX_LOCK   EnterCriticalSection(&g_allocMutex);
#define MUTEX_UNLOCK LeaveCriticalSection(&g_allocMutex);

#endif // host os
#else // not host

#include "multi_core/mutex.h"

static Core_Mutex g_allocMutex;
#define MUTEX_CREATE
#define MUTEX_DESTROY
#define MUTEX_LOCK   MultiCore_LockRecursiveMutex(&g_allocMutex);
#define MUTEX_UNLOCK MultiCore_UnlockRecursiveMutex(&g_allocMutex);

#endif // end CPU_host
// ---------------------------------------------------------------------------------------------------------------------------------
// Originally created on 12/22/2000 by Paul Nettle
//
// Copyright 2000, Fluid Studios, Inc., all rights reserved.
// ---------------------------------------------------------------------------------------------------------------------------------
#define REPORTED_ADDRESS_BITS_MASK(x) (((uintptr_t)(x)) & 0xF)
#define REPORTED_ADDRESS_BITES_SAME_AS_REPORTED 0x1

#define CLEAN_REPORTED_ADDRESS(x) (void*)(((uintptr_t)(x)) & ~0xF)

typedef struct AllocUnit {
	void *uncleanReportedAddress; //address is always at least 16 byte aligned so we stuff things in the bottom four bit!
	char const *sourceFile;
	char const *sourceFunc;
	struct AllocUnit *next;
	struct AllocUnit *prev;

	uint64_t allocationNumber;

	uint32_t reportedSize; // as most allocs will be less 4GiB we assume we saturate to 4GiB should be enough to spot
	uint32_t sourceLine;

} AllocUnit;

#define hashBits 12u
#define hashSize (1u << hashBits)
static AllocUnit *hashTable[hashSize];
static AllocUnit *reservoir;
static AllocUnit **reservoirBuffer = NULL;
static uint32_t reservoirBufferSize = 0;
GLOBAL_HEAP_ALLOCATOR(reservoirAllocator)

void Memory_TrackerInit() {
#if MEMORY_TRACKING == 1
	Memory_HeapAllocatorInit(reservoirAllocator);
#endif
}

void Memory_TrackerFinish() {
#if MEMORY_TRACKING == 1
	Memory_HeapAllocatorFinish(reservoirAllocator);
#endif
}

ALWAYS_INLINE size_t calculateReportedSize(const size_t actualSize) {
	return actualSize - Memory_TrackingPaddingSize * sizeof(uint32_t) * 2;
}

ALWAYS_INLINE void * Memory_TrackerCalculateActualAddress(const void *reportedAddress) {
	// We allow this...
	if (!reportedAddress) {
		return NULL;
	}
	if( REPORTED_ADDRESS_BITS_MASK(reportedAddress) & REPORTED_ADDRESS_BITES_SAME_AS_REPORTED) {
		return (void*) (((uintptr_t)reportedAddress) & ~0xF);
	}

	// JUst account for the padding
	return (void *) (((uint8_t const *) (reportedAddress)) - sizeof(uint32_t) * Memory_TrackingPaddingSize);
}

ALWAYS_INLINE void *calculateReportedAddress(const void *actualAddress) {
	// We allow this...
	if (!actualAddress) {
		return NULL;
	}

	// JUst account for the padding
	return (void *) (((uint8_t const *) (actualAddress)) + sizeof(uint32_t) * Memory_TrackingPaddingSize);
}

static const char *sourceFileStripper(const char *sourceFile) {
	char const* ptr = sourceFile + utf8size(sourceFile);
	uint32_t slashCount = 0;
	while(ptr > sourceFile) {
		if(*ptr == '\\' || *ptr == '/') {
			slashCount++;
			if(slashCount == 3) {
				return ptr + 1;
			}
		}
		ptr--;
	}
	return sourceFile;
}

static AllocUnit *findAllocUnit(const void *reportedAddress) {
	// Just in case...
	assert(reportedAddress != NULL);

	// Use the address to locate the hash index. Note that we shift off the lower four bits. This is because most allocated
	// addresses will be on four-, eight- or even sixteen-byte boundaries. If we didn't do this, the hash index would not have
	// very good coverage.
	uintptr_t hashIndex = (((uintptr_t) reportedAddress) >> 4) & (hashSize - 1);
	AllocUnit *ptr = hashTable[hashIndex];
	while (ptr) {
		if (CLEAN_REPORTED_ADDRESS(ptr->uncleanReportedAddress) == reportedAddress) {
			return ptr;
		}
		ptr = ptr->next;
	}

	return NULL;
}

static bool GrowReservoir() {
	debug_print(ANSI_BRIGHT_ON "Memory Tracker: Growing Reservoir" ANSI_BRIGHT_OFF "\n");

	// Allocate 256 reservoir elements
	reservoir = (AllocUnit *) reservoirAllocator->calloc(reservoirAllocator, 256, sizeof(AllocUnit));
	// Danger Will Robinson!
	if (reservoir == NULL) {
		return false;
	}

	// Build a linked-list of the elements in our reservoir
	for (unsigned int i = 0; i < 256 - 1; i++) {
		reservoir[i].next = &reservoir[i + 1];
	}

	// Add this address to our reservoirBuffer so we can free it later
	AllocUnit **temp = (AllocUnit **) reservoirAllocator->realloc(reservoirAllocator, reservoirBuffer, (reservoirBufferSize + 1) * sizeof(AllocUnit *));
	assert(temp);
	if (temp) {
		reservoirBuffer = temp;
		reservoirBuffer[reservoirBufferSize++] = reservoir;
	}

	return true;
}

void * Memory_TrackedMalloc(Memory_Allocator * allocator_,
													char const* sourceFile_,
													unsigned int const sourceLine_,
													char const * sourceFunc_,
													size_t const size_) {
	void * mem = allocator_->malloc(allocator_, Memory_TrackerCalculateActualSize(size_));

	if (mem == NULL) {
		debug_print("ERROR: Request for allocation failed. Out of memory.");
		return NULL;
	}
#if MEMORY_TRACKING == 0
	return mem;
#endif
	if (reservoirBufferSize == 0) {
		MUTEX_CREATE
	}

	MUTEX_LOCK
	// If necessary, grow the reservoir of unused allocation units
	if (!reservoir) {
			if (!GrowReservoir()) {
				MUTEX_UNLOCK
				return NULL;
		}
	}

	if (Memory_TrackerBreakOnAllocNumber != 0 && Memory_TrackerBreakOnAllocNumber == g_allocCounter + 1) {
		debug_print("WARNING:  Break on allocation number hit");
		IKUY_DEBUG_BREAK();
	}

	if(sourceFile_ == NULL) {
		debug_print("WARNING:  Allocation without tracking file/line/function info");
		IKUY_DEBUG_BREAK();
	}

	// Logical flow says this should never happen...
	assert(reservoir != NULL);

	// Grab a new allocaton unit from the front of the reservoir
	AllocUnit *au = reservoir;
	reservoir = au->next;

	// Populate it with some real data
	memset(au, 0, sizeof(AllocUnit));
	au->reportedSize = (size_ > 0xFFFFFFFF) ? (uint32_t)(0xFFFFFFFF) : (uint32_t) size_;
	au->uncleanReportedAddress = calculateReportedAddress(mem);
	au->sourceFile = sourceFile_;
	au->sourceLine = sourceLine_;
	au->sourceFunc = sourceFunc_;
	au->allocationNumber = ++g_allocCounter;

	// Insert the new allocation into the hash table
	uintptr_t hashIndex = (((uintptr_t) au->uncleanReportedAddress) >> 4) & (hashSize - 1);
	if (hashTable[hashIndex]) {
		hashTable[hashIndex]->prev = au;
	}
	au->next = hashTable[hashIndex];
	au->prev = NULL;
	hashTable[hashIndex] = au;

	MUTEX_UNLOCK

	return CLEAN_REPORTED_ADDRESS(au->uncleanReportedAddress);
}

void * Memory_TrackedAalloc(Memory_Allocator * allocator_,
													 char const * sourceFile_,
													 unsigned int const sourceLine_,
													 char const * sourceFunc_,
													 size_t const size_,
													 size_t const align_) {
	void *mem = allocator_->aalloc(allocator_, Memory_TrackerCalculateActualSize(size_), align_);

	if (mem == NULL) {
		debug_print("ERROR: Request for allocation failed. Out of memory.");
		return NULL;
	}
#if MEMORY_TRACKING == 0
	return mem;
#endif

	if (reservoirBufferSize == 0) {
		MUTEX_CREATE
	}

	MUTEX_LOCK

	if(Memory_TrackerBreakOnAllocNumber != 0 && Memory_TrackerBreakOnAllocNumber == g_allocCounter+1) {
		debug_print("WARNING: Break on allocation number hit");
		IKUY_DEBUG_BREAK();
	}

	if(sourceFile_ == NULL) {
		debug_print("WARNING: Allocation without tracking file/line/function info");
		IKUY_DEBUG_BREAK();
	}

	// If necessary, grow the reservoir of unused allocation units
	if (!reservoir) {
		if (!GrowReservoir()) {
			MUTEX_UNLOCK
			return NULL;
		}
	}

	// Logical flow says this should never happen...
	assert(reservoir != NULL);

	// Grab a new allocaton unit from the front of the reservoir
	AllocUnit *au = reservoir;
	reservoir = au->next;

	// Populate it with some real data
	memset(au, 0, sizeof(AllocUnit));
	au->reportedSize = (size_ > 0xFFFFFFFF) ? (uint32_t)(0xFFFFFFFF) : (uint32_t)size_;
	au->uncleanReportedAddress = mem;
	au->sourceFile = sourceFile_;
	au->sourceLine = sourceLine_;
	au->sourceFunc = sourceFunc_;
	au->allocationNumber = ++g_allocCounter;

	// or in reported == allocated bit
	au->uncleanReportedAddress = (void*)(((uintptr_t)au->uncleanReportedAddress) | REPORTED_ADDRESS_BITES_SAME_AS_REPORTED);

	// Insert the new allocation into the hash table
	uintptr_t hashIndex = (((uintptr_t) au->uncleanReportedAddress) >> 4) & (hashSize - 1);
	if (hashTable[hashIndex]) {
		hashTable[hashIndex]->prev = au;
	}
	au->next = hashTable[hashIndex];
	au->prev = NULL;
	hashTable[hashIndex] = au;

	MUTEX_UNLOCK

	return CLEAN_REPORTED_ADDRESS(au->uncleanReportedAddress);
}
void * Memory_TrackedCalloc(Memory_Allocator * allocator_,
                                     char const * sourceFile_,
                                     unsigned int sourceLine_,
                                     char const * sourceFunc_,
                                     size_t count_,
                                     size_t size_) {
	void * mem = Memory_TrackedMalloc(allocator_, sourceFile_, sourceLine_,sourceFunc_, count_ * size_);
	if(mem) {
		memset(mem, 0, count_ * size_);
	}
	return mem;
}


void *Memory_TrackedRealloc(Memory_Allocator * allocator_,
														char const * sourceFile_,
														unsigned int const sourceLine_,
														char const * sourceFunc_,
														void * address_,
														size_t const size_) {
	// Calling realloc with a nullptr should force same operations as a malloc
	if (!address_) {
		return Memory_TrackedMalloc(allocator_, sourceFile_, sourceLine_, sourceFunc_, size_);
	}
	void *mem = allocator_->realloc(allocator_, Memory_TrackerCalculateActualAddress(address_), Memory_TrackerCalculateActualSize(size_));

	if (!mem) {
		debug_print("ERROR: Request for reallocation failed. Out of memory.");
		return NULL;
	}

#if MEMORY_TRACKING == 0
		return mem;
#endif

	MUTEX_LOCK

	if(Memory_TrackerBreakOnAllocNumber != 0 && Memory_TrackerBreakOnAllocNumber == g_allocCounter+1) {
		debug_print("WARNING: Break on allocation number hit");
		IKUY_DEBUG_BREAK();
	}

	if(sourceFile_ == NULL) {
		debug_print("WARNING: Allocation without tracking file/line/function info");
		IKUY_DEBUG_BREAK();
	}

	// Locate the existing allocation unit
	AllocUnit *au = findAllocUnit(address_);

	// If you hit this assert, you tried to reallocate RAM that wasn't allocated by this memory manager.
	if (au == nullptr) {
		debug_print("ERROR: Request to reallocate RAM that was never allocated");
		MUTEX_UNLOCK
		return nullptr;
	}

	// Do the reallocation
	void *oldReportedAddress = address_;
	size_t newActualSize = Memory_TrackerCalculateActualSize(size_);

	// Update the allocation with the new information
	au->reportedSize = (calculateReportedSize(newActualSize) > 0xFFFFFFFF) ? (uint32_t)(0xFFFFFFFF) : (uint32_t)calculateReportedSize(size_);
	au->uncleanReportedAddress = calculateReportedAddress(mem);
	au->sourceFile = sourceFile_;
	au->sourceLine = sourceLine_;
	au->sourceFunc = sourceFunc_;
	au->allocationNumber = ++g_allocCounter;

	// The reallocation may cause the address to change, so we should relocate our allocation unit within the hash table
	unsigned int hashIndex = ~0;
	if (oldReportedAddress != CLEAN_REPORTED_ADDRESS(au->uncleanReportedAddress)) {
		// Remove this allocation unit from the hash table
		{
			uintptr_t hashIndex = (((uintptr_t) address_) >> 4) & (hashSize - 1);
			if (hashTable[hashIndex] == au) {
				hashTable[hashIndex] = hashTable[hashIndex]->next;
			} else {
				if (au->prev) {
					au->prev->next = au->next;
				}
				if (au->next) {
					au->next->prev = au->prev;
				}
			}
		}

		// Re-insert it back into the hash table
		hashIndex = (((uintptr_t) au->uncleanReportedAddress) >> 4) & (hashSize - 1);
		if (hashTable[hashIndex]) {
			hashTable[hashIndex]->prev = au;
		}
		au->next = hashTable[hashIndex];
		au->prev = NULL;
		hashTable[hashIndex] = au;
	}

	MUTEX_UNLOCK

	// Return the (reported) address of the new allocation unit
	return CLEAN_REPORTED_ADDRESS(au->uncleanReportedAddress);
}

bool Memory_TrackedFree(Memory_Allocator * allocator_, void * address_) {
	if (!address_) return true;

#if MEMORY_TRACKING == 0
	allocator_->free(allocator_, address_);
	return true;
#endif

	if (reservoirBufferSize == 0 || reservoirBuffer == NULL) {
		debug_print("ERROR: Free before any allocations have occured or after exit!");
		return true; // we can't tell if this is an aalloc or other assume other as more common...
	}

	MUTEX_LOCK

	// Go get the allocation unit
	AllocUnit *au = findAllocUnit(address_);
	if (au == NULL) {
		debug_printf("ERROR: Request to deallocate RAM (%p) that was never allocated\n", address_);
		IKUY_DEBUG_BREAK();
		MUTEX_UNLOCK
		return false;
	}
	bool const adjustPtr = (REPORTED_ADDRESS_BITS_MASK(au->uncleanReportedAddress) & REPORTED_ADDRESS_BITES_SAME_AS_REPORTED) == 0;

	// Wipe the deallocated RAM with a new pattern. This doen't actually do us much good in debug mode under WIN32,
	// because Microsoft's memory debugging & tracking utilities will wipe it right after we do. Oh well.

	//	wipeWithPattern(au, releasedPattern);

	// Remove this allocation unit from the hash table
	uintptr_t hashIndex = (((uintptr_t) au->uncleanReportedAddress) >> 4) & (hashSize - 1);
	if (hashTable[hashIndex] == au) {
		hashTable[hashIndex] = au->next;
	} else {
		if (au->prev) {
			au->prev->next = au->next;
		}
		if (au->next) {
			au->next->prev = au->prev;
		}
	}

	// Add this allocation unit to the front of our reservoir of unused allocation units
	memset(au, 0, sizeof(AllocUnit));
	au->next = reservoir;
	reservoir = au;

	MUTEX_UNLOCK

	if (adjustPtr) {
		allocator_->free(allocator_, Memory_TrackerCalculateActualAddress(address_));
	} else {
		allocator_->free(allocator_, address_);
	}
	return adjustPtr;
}

void Memory_TrackerDestroyAndLogLeaks() {
	MUTEX_LOCK
	bool loggedHeader = 0;
	for (int i = 0; i < hashSize; ++i) {
		AllocUnit *au = hashTable[i];
		while (au != NULL) {
			if (loggedHeader == false) {
				loggedHeader = true;
				debug_printf("-=-=-=-=-=-=- Memory Leak Report -=-=-=-=-=-=-\n");
			}
			if(au->sourceFile) {
				char const *fileNameOnly = sourceFileStripper( au->sourceFile );
				debug_printf( "%u bytes from %s(%u): %s number: %lu\n", au->reportedSize, fileNameOnly, au->sourceLine, au->sourceFunc, au->allocationNumber );
			} else
				debug_printf( "%u bytes from an unknown caller number: %lu\n", au->reportedSize, au->allocationNumber );

			au = au->next;
		}
	}
	if(loggedHeader == false) {
		debug_printf("-=-=-=-=-=-=- No Memory Leaks! -=-=-=-=-=-=-\n");
	}

	// free the reservoirs
	for(uint32_t i = 0;i < reservoirBufferSize;++i) {
		reservoirAllocator->free(reservoirAllocator, reservoirBuffer[i]);
	}
	reservoirBuffer = NULL;
	reservoir = NULL;
	reservoirBufferSize = 0;

	memset(hashTable, 0, sizeof(AllocUnit*) * hashSize);
	MUTEX_UNLOCK

	MUTEX_DESTROY
}

#else

void Memory_TrackerDestroyAndLogLeaks() {}

#endif


void Memory_MallocInit() {
#if MEMORY_TRACKING_SETUP == 1
	Memory_TrackerInit();
#endif
}

void Memory_MallocFinish() {
#if MEMORY_TRACKING_SETUP == 1
	Memory_TrackerFinish();
#endif
}
