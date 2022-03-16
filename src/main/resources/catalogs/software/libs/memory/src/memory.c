// License Summary: MIT see LICENSE file
#include "core/core.h"
#include "memory/memory.h"

#include "dbg/assert.h"
#include "dbg/print.h"
#include "multi_core/core_local.h"
#include "core/utf8.h"

static CORE_LOCAL(char const*, g_lastSourceFile);
static CORE_LOCAL(unsigned int, g_lastSourceLine);
static CORE_LOCAL(char const*, g_lastSourceFunc);
static uint64_t g_allocCounter = 0;
uint64_t Memory_TrackerBreakOnAllocNumber = 0; // set this here or in code before the allocation occurs to break

// #define MEMORY_TRACKING 0 will switch off the cost of tracking bar 3 TLS pushing and memory for the strings
// #define MEMORY_TRACKING_SETUP 0 in header will remove this overhead as well..

#if !defined(MEMORY_TRACKING) && (defined(MEMORY_TRACKING_SETUP) && MEMORY_TRACKING_SETUP != 0)
#define MEMORY_TRACKING 1
#endif

#if MEMORY_TRACKING == 1 && (!defined(MEMORY_TRACKING_SETUP) && MEMORY_TRACKING_SETUP == 0)
#error MEMORY_TRACKING requires MEMORY_TRACKING_SETUP == 1
#endif

bool Memory_TrackerPushNextSrcLoc(const char *sourceFile,
                             const unsigned int sourceLine,
                             const char *sourceFunc) {
	WRITE_CORE_LOCAL(g_lastSourceFile, sourceFile);
	WRITE_CORE_LOCAL(g_lastSourceLine, sourceLine);
	WRITE_CORE_LOCAL(g_lastSourceFunc, sourceFunc);
  return true;
}


#if MEMORY_TRACKING == 1

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

ALWAYS_INLINE size_t calculateReportedSize(const size_t actualSize) {
	return actualSize - Memory_TrackingPaddingSize * sizeof(uint32_t) * 2;
}

ALWAYS_INLINE void *Memory_TrackerCalculateActualAddress(const void *reportedAddress) {
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
	// Allocate 256 reservoir elements
	reservoir = (AllocUnit *) platformCalloc(256, sizeof(AllocUnit));
	// Danger Will Robinson!
	if (reservoir == NULL) {
		return false;
	}

	// Build a linked-list of the elements in our reservoir
	for (unsigned int i = 0; i < 256 - 1; i++) {
		reservoir[i].next = &reservoir[i + 1];
	}

	// Add this address to our reservoirBuffer so we can free it later
	AllocUnit **temp = (AllocUnit **) platformRealloc(reservoirBuffer, (reservoirBufferSize + 1) * sizeof(AllocUnit *));
	assert(temp);
	if (temp) {
		reservoirBuffer = temp;
		reservoirBuffer[reservoirBufferSize++] = reservoir;
	}

	return true;
}

void *Memory_TrackedAlloc(const char *sourceFile,
													const unsigned int sourceLine,
													const char *sourceFunc,
													const size_t reportedSize,
													void *actualSizedAllocation) {
	if (actualSizedAllocation == NULL) {
		debug_print("ERROR: Request for allocation failed. Out of memory.");
		return NULL;
	}

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

	if(sourceFile == NULL) {
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
	au->reportedSize = (reportedSize > 0xFFFFFFFF) ? (uint32_t)(0xFFFFFFFF) : (uint32_t) reportedSize;
	au->uncleanReportedAddress = calculateReportedAddress(actualSizedAllocation);
	au->sourceFile = sourceFile;
	au->sourceLine = sourceLine;
	au->sourceFunc = sourceFunc;
	au->allocationNumber = ++g_allocCounter;

	// Insert the new allocation into the hash table
	uintptr_t hashIndex = (((uintptr_t) au->uncleanReportedAddress) >> 4) & (hashSize - 1);
	if (hashTable[hashIndex]) {
		hashTable[hashIndex]->prev = au;
	}
	au->next = hashTable[hashIndex];
	au->prev = NULL;
	hashTable[hashIndex] = au;

	WRITE_CORE_LOCAL(g_lastSourceFile, nullptr);
	WRITE_CORE_LOCAL(g_lastSourceLine, 0);
	WRITE_CORE_LOCAL(g_lastSourceFunc, nullptr);

	MUTEX_UNLOCK

	return CLEAN_REPORTED_ADDRESS(au->uncleanReportedAddress);
}

void *Memory_TrackedAAlloc(const char *sourceFile,
													 const unsigned int sourceLine,
													 const char *sourceFunc,
													 const size_t reportedSize,
													 void *actualSizedAllocation) {
	if (actualSizedAllocation == NULL) {
		debug_print("ERROR: Request for allocation failed. Out of memory.");
		return NULL;
	}

	if (reservoirBufferSize == 0) {
		MUTEX_CREATE
	}

	MUTEX_LOCK

	if(Memory_TrackerBreakOnAllocNumber != 0 && Memory_TrackerBreakOnAllocNumber == g_allocCounter+1) {
		debug_print("WARNING: Break on allocation number hit");
		IKUY_DEBUG_BREAK();
	}

	if(sourceFile == NULL) {
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
	au->reportedSize = (reportedSize > 0xFFFFFFFF) ? (uint32_t)(0xFFFFFFFF) : (uint32_t)reportedSize;
	au->uncleanReportedAddress = actualSizedAllocation;
	au->sourceFile = sourceFile;
	au->sourceLine = sourceLine;
	au->sourceFunc = sourceFunc;
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

	WRITE_CORE_LOCAL(g_lastSourceFile, nullptr);
	WRITE_CORE_LOCAL(g_lastSourceLine, 0);
	WRITE_CORE_LOCAL(g_lastSourceFunc, nullptr);

	MUTEX_UNLOCK

	return CLEAN_REPORTED_ADDRESS(au->uncleanReportedAddress);
}

void *Memory_TrackedRealloc(const char *sourceFile,
														const unsigned int sourceLine,
														const char *sourceFunc,
														const size_t reportedSize,
														void *reportedAddress,
														void *actualSizedAllocation) {
	// Calling realloc with a NULL should force same operations as a malloc
	if (!reportedAddress) {
		return Memory_TrackedAlloc(sourceFile, sourceLine, sourceFunc, reportedSize, actualSizedAllocation);
	}

	if (!actualSizedAllocation) {
		debug_print("ERROR: Request for reallocation failed. Out of memory.");
		return NULL;
	}

	MUTEX_LOCK

	if(Memory_TrackerBreakOnAllocNumber != 0 && Memory_TrackerBreakOnAllocNumber == g_allocCounter+1) {
		debug_print("WARNING: Break on allocation number hit");
		IKUY_DEBUG_BREAK();
	}

	if(sourceFile == NULL) {
		debug_print("WARNING: Allocation without tracking file/line/function info");
		IKUY_DEBUG_BREAK();
	}

	// Locate the existing allocation unit
	AllocUnit *au = findAllocUnit(reportedAddress);

	// If you hit this assert, you tried to reallocate RAM that wasn't allocated by this memory manager.
	if (au == NULL) {
		debug_print("ERROR: Request to reallocate RAM that was never allocated");
		MUTEX_UNLOCK
		return NULL;
	}

	// Do the reallocation
	void *oldReportedAddress = reportedAddress;
	size_t newActualSize = Memory_TrackerCalculateActualSize(reportedSize);

	// Update the allocation with the new information
	au->reportedSize = (calculateReportedSize(newActualSize) > 0xFFFFFFFF) ? (uint32_t)(0xFFFFFFFF) : (uint32_t)calculateReportedSize(newActualSize);
	au->uncleanReportedAddress = calculateReportedAddress(actualSizedAllocation);
	au->sourceFile = sourceFile;
	au->sourceLine = sourceLine;
	au->sourceFunc = sourceFunc;
	au->allocationNumber = ++g_allocCounter;

	// The reallocation may cause the address to change, so we should relocate our allocation unit within the hash table
	unsigned int hashIndex = ~0;
	if (oldReportedAddress != CLEAN_REPORTED_ADDRESS(au->uncleanReportedAddress)) {
		// Remove this allocation unit from the hash table
		{
			uintptr_t hashIndex = (((uintptr_t) reportedAddress) >> 4) & (hashSize - 1);
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


	// Prepare the allocation unit for use (wipe it with recognizable garbage)
	//	wipeWithPattern(au, unusedPattern, originalReportedSize);

	WRITE_CORE_LOCAL(g_lastSourceFile, nullptr);
	WRITE_CORE_LOCAL(g_lastSourceLine, 0);
	WRITE_CORE_LOCAL(g_lastSourceFunc, nullptr);

	MUTEX_UNLOCK

	// Return the (reported) address of the new allocation unit
	return CLEAN_REPORTED_ADDRESS(au->uncleanReportedAddress);
}

bool Memory_TrackedFree(const void *reportedAddress) {
	if (!reportedAddress) {
		return false;
	}

	if (reservoirBufferSize == 0 || reservoirBuffer == NULL) {
		debug_print("ERROR: Free before any allocations have occured or after exit!");
		return true; // we can't tell if this is an aalloc or other assume other as more common...
	}

	MUTEX_LOCK

	// Go get the allocation unit
	AllocUnit *au = findAllocUnit(reportedAddress);
	if (au == NULL) {
		debug_print("ERROR: Request to deallocate RAM that was never allocated");
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

	return adjustPtr;
}

void *trackedMalloc(Memory_Allocator * allocator, size_t size) {
	if(READ_CORE_LOCAL(g_lastSourceFile) == nullptr) {
		debug_print("g_lastSourceFile == nullptr\n");
	}
	void *mem = platformMalloc(Memory_TrackerCalculateActualSize(size));
	if(g_lastSourceFile == nullptr) {
		debug_print("g_lastSourceFile == nullptr\n");
		return mem;
	}
	return Memory_TrackedAlloc(g_lastSourceFile, g_lastSourceLine, g_lastSourceFunc, size, mem);
}

void *trackedAalloc(Memory_Allocator * allocator, size_t size, size_t align) {
	if( align <= 16) {
		return trackedMalloc(allocator, size);
	}
	void *mem = platformAalloc(Memory_TrackerCalculateActualSize(size), align);
	return Memory_TrackedAAlloc(g_lastSourceFile, g_lastSourceLine, g_lastSourceFunc, size, mem);
}

void *trackedCalloc(Memory_Allocator * allocator, size_t count, size_t size) {
	void *mem = platformMalloc(Memory_TrackerCalculateActualSize(count * size));
	if (mem) {
		memset(mem, 0, Memory_TrackerCalculateActualSize(count * size));
	}

	return Memory_TrackedAlloc(g_lastSourceFile, g_lastSourceLine, g_lastSourceFunc, count * size, mem);
}

void *trackedRealloc(Memory_Allocator * allocator, void *ptr, size_t size) {
	void *mem = platformRealloc(Memory_TrackerCalculateActualAddress(ptr), Memory_TrackerCalculateActualSize(size));
	return Memory_TrackedRealloc(g_lastSourceFile, g_lastSourceLine, g_lastSourceFunc, size, ptr, mem);
}

void trackedFree(Memory_Allocator * allocator, void *ptr) {
	bool const adjustPtr = Memory_TrackedFree(ptr);
	if (adjustPtr) {
		platformFree(Memory_TrackerCalculateActualAddress(ptr));
	} else {
		platformFree(ptr);
	}
}

Memory_Allocator Memory_GlobalAllocator = {
		.malloc = &trackedMalloc,
		.aalloc = &trackedAalloc,
		.calloc = &trackedCalloc,
		.realloc = &trackedRealloc,
		.free = &trackedFree
};


void Memory_TrackerDestroyAndLogLeaks() {
	MUTEX_LOCK
	bool loggedHeader = 0;
	for (int i = 0; i < hashSize; ++i) {
		AllocUnit *au = hashTable[i];
		while (au != NULL) {
			if (loggedHeader == false) {
				loggedHeader = true;
				debug_printf("INFO: -=-=-=-=-=-=- Memory Leak Report -=-=-=-=-=-=-");
			}
			if(au->sourceFile) {
				char const *fileNameOnly = sourceFileStripper(au->sourceFile);
				debug_printf("INFO: %u bytes from %s(%u): %s number: %lu", au->reportedSize, fileNameOnly, au->sourceLine, au->sourceFunc, au->allocationNumber);
			} else {
				debug_printf("INFO: %u bytes from an unknown caller number: %lu", au->reportedSize, au->allocationNumber);
			}
			au = au->next;
		}
	}

	// free the reservoirs
	for(uint32_t i = 0;i < reservoirBufferSize;++i) {
		platformFree(reservoirBuffer[i]);
	}
	reservoirBuffer = NULL;
	reservoir = NULL;
	reservoirBufferSize = 0;

	memset(hashTable, 0, sizeof(AllocUnit*) * hashSize);
	MUTEX_UNLOCK

	MUTEX_DESTROY
}

#else

Memory_Allocator Memory_GlobalAllocator = {
        &platformMalloc,
        &platformAalloc,
        &platformCalloc,
        &platformRealloc,
        &platformFree
};
void Memory_TrackerDestroyAndLogLeaks() {}

void *Memory_TrackedAlloc(const char *a, const unsigned int b, const char *c, const size_t d, void *e) {
    debug_print("ERROR: Memory_TrackedAlloc called in non tracking build");
    return NULL;
};
void *Memory_TrackedAAlloc(const char *a, const unsigned int b, const char *c, const size_t d, void *e) {
    debug_print("ERROR: Memory_TrackedAAlloc called in non tracking build");
    return NULL;
}
void *Memory_TrackedRealloc(const char *a,
                                           const unsigned int b,
                                           const char *c,
                                           const size_t d,
                                           void *e,
                                           void *f) {
    debug_print("ERROR: Memory_TrackedRealloc called in non tracking build");
    return NULL;
}
#endif