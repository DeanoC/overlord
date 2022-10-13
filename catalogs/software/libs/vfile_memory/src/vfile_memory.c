#include "core/core.h"
#include "vfile/vfile.h"
#include "vfile/utils.h"
#include "vfile_memory/memory.h"
#include "memory/memory.h"

static const uint32_t VFileMemory_HeaderSize = sizeof(VFile_Interface_t) + sizeof(VFileMemory_Data_t);

static void VFile_MemFile_Close(VFile_Interface_t *vif) {
	if(vif == nullptr) return;

	VFileMemory_Data_t *vof = (VFileMemory_Data_t *) (vif + 1);
  if (vof->autoFreeMemoryOnClose) {
		MFREE(vof->allocator, (void*)vof->memory);
  }

	MFREE(vof->allocator, vif);
}

static void VFile_MemFile_Flush(VFile_Interface_t *vof) {
  // do nothing
}

static size_t VFile_MemFile_Read(VFile_Interface_t *vif, void *buffer, size_t byteCount) {
	VFileMemory_Data_t *vof = (VFileMemory_Data_t *) (vif + 1);
  size_t size = byteCount;

  if (vof->offset + byteCount >= vof->size) {
    size = vof->size - vof->offset;
    if (size < 0 || size > vof->size) {
      return 0;
    }
  }
  memcpy(buffer, ((uint8_t *) vof->memory) + vof->offset, size);
  vof->offset += size;
  return size;
}

static size_t VFile_MemFile_Write(VFile_Interface_t *vif, void const *buffer, size_t byteCount) {
  VFileMemory_Data_t *vof = (VFileMemory_Data_t *) (vif + 1);
  size_t size = byteCount;

  if (vof->offset + byteCount >= vof->size) {
    if (!vof->autoFreeMemoryOnClose) {
      size = vof->size - vof->offset;
      if (size < 0 || size > vof->size) {
        return 0;
      }
    } else {
      // grow the memory to fit
      vof->size = vof->offset + byteCount;
      vof->memory = MREALLOC(vof->allocator, (void*)vof->memory, vof->size);
    }
  }

  memcpy(((uint8_t *) vof->memory) + vof->offset, buffer, size);
  vof->offset += size;
  return size;
}

static bool VFile_MemFile_Seek(VFile_Interface_t *vif, int64_t offset, enum VFile_SeekDir origin) {
  VFileMemory_Data_t *vof = (VFileMemory_Data_t *) (vif + 1);

  size_t voff = 0;
  switch (origin) {
    case VFile_SD_Begin: voff = 0;
      break;
    case VFile_SD_Current: voff = vof->offset;
      break;
    case VFile_SD_End: voff = vof->size;
      break;
    default:return false;
  }

  if (voff + offset < 0) {
    vof->offset = 0;
    return false;
  } else if (voff + offset < vof->size) {
    vof->offset = voff + offset;
    return true;
  } else {
    vof->offset = vof->size;
    return false;
  }
}

static int64_t VFileMemory_tell(VFile_Interface_t *vif) {
  VFileMemory_Data_t *vof = (VFileMemory_Data_t *) (vif + 1);
  return (int64_t) vof->offset;
}

static size_t VFile_MemFile_Size(VFile_Interface_t *vif) {
  VFileMemory_Data_t *vof = (VFileMemory_Data_t *) (vif + 1);
  return vof->size;
}

static char const *VFile_MemFile_GetName(VFile_Interface_t *vif) {
  static char const NoName[] = "*NO_NAME*";
  return NoName;
}

static bool VFile_MemFile_IsEOF(VFile_Interface_t *vif) {
  VFileMemory_Data_t *vof = (VFileMemory_Data_t *) (vif + 1);
  return vof->offset >= vof->size;
}
#if MEMORY_TRACKING_SETUP == 1
#undef VFileMemory_FromBuffer
#undef VFileMemory_FromSize
#undef VFileMemory_CreateEmpty
#endif

VFile_Handle VFileMemory_FromBuffer(void const *memory, size_t size, bool takeOwnership, Memory_Allocator* memoryAllocator) {

#if MEMORY_TRACKING_SETUP == 1
	// call the allocator direct, so that the line and file comes free the caller
	VFile_Interface_t *vif = (VFile_Interface_t *) memoryAllocator->malloc(memoryAllocator, VFileMemory_HeaderSize);
#else
	VFile_Interface_t *vif = (VFile_Interface_t *) MALLOC(memoryAllocator, VFileMemory_HeaderSize);
#endif

	VFileMemory_Data_t *vof = (VFileMemory_Data_t *) (vif + 1);
  vif->magic = InterfaceMagic;
  vif->type = VFILE_MAKE_ID('M', 'E', 'M', ' ');
  vif->closeFunc = &VFile_MemFile_Close;
  vif->flushFunc = &VFile_MemFile_Flush;
  vif->readFunc = &VFile_MemFile_Read;
  vif->writeFunc = &VFile_MemFile_Write;
  vif->seekFunc = &VFile_MemFile_Seek;
  vif->tellFunc = &VFileMemory_tell;
  vif->sizeFunc = &VFile_MemFile_Size;
  vif->nameFunc = &VFile_MemFile_GetName;
  vif->isEofFunc = &VFile_MemFile_IsEOF;
  vof->memory = memory;
  vof->allocator = memoryAllocator;
  vof->size = size;
  vof->autoFreeMemoryOnClose = takeOwnership;
  vof->offset = 0;

  return (VFile_Handle) vif;
}

VFile_Handle VFileMemory_FromSize(size_t size, Memory_Allocator* memoryAllocator) {

	size_t sizeWithHeader = VFileMemory_HeaderSize + size;
#if MEMORY_TRACKING_SETUP == 1
	// call the allocator direct, so that the line and file comes free the caller
	VFile_Interface_t *vif = (VFile_Interface_t *) memoryAllocator->malloc(memoryAllocator, sizeWithHeader);
#else
	VFile_Interface_t *vif = (VFile_Interface_t *) MALLOC(memoryAllocator, sizeWithHeader);
#endif

	VFileMemory_Data_t *vof = (VFileMemory_Data_t *) (vif + 1);
  vif->magic = InterfaceMagic;
  vif->type = 1;// TODO VFile_Type_Memory;
  vif->closeFunc = &VFile_MemFile_Close;
  vif->flushFunc = &VFile_MemFile_Flush;
  vif->readFunc = &VFile_MemFile_Read;
  vif->writeFunc = &VFile_MemFile_Write;
  vif->seekFunc = &VFile_MemFile_Seek;
  vif->tellFunc = &VFileMemory_tell;
  vif->sizeFunc = &VFile_MemFile_Size;
  vif->nameFunc = &VFile_MemFile_GetName;
  vif->isEofFunc = &VFile_MemFile_IsEOF;
  vof->memory = (vof + 1);
	vof->allocator = memoryAllocator;
  vof->size = size;
  vof->autoFreeMemoryOnClose = false; // single block used
  vof->offset = 0;

  return (VFile_Handle) vif;
}
VFile_Handle VFileMemory_CreateEmpty(Memory_Allocator* memoryAllocator) {

	size_t sizeWithHeader = VFileMemory_HeaderSize;
#if MEMORY_TRACKING_SETUP == 1
	// call the allocator direct, so that the line and file comes free the caller
	VFile_Interface_t *vif = (VFile_Interface_t *) memoryAllocator->malloc(memoryAllocator, sizeWithHeader);
#else
	VFile_Interface_t *vif = (VFile_Interface_t *) MALLOC(memoryAllocator, sizeWithHeader);
#endif

	VFileMemory_Data_t *vof = (VFileMemory_Data_t *) (vif + 1);
	vif->magic = InterfaceMagic;
	vif->type = VFILE_MAKE_ID('M', 'E', 'M', '_');
	vif->closeFunc = &VFile_MemFile_Close;
	vif->flushFunc = &VFile_MemFile_Flush;
	vif->readFunc = &VFile_MemFile_Read;
	vif->writeFunc = &VFile_MemFile_Write;
	vif->seekFunc = &VFile_MemFile_Seek;
	vif->tellFunc = &VFileMemory_tell;
	vif->sizeFunc = &VFile_MemFile_Size;
	vif->nameFunc = &VFile_MemFile_GetName;
	vif->isEofFunc = &VFile_MemFile_IsEOF;
	vof->memory = nullptr;
	vof->allocator = memoryAllocator;
	vof->size = 0;
	vof->autoFreeMemoryOnClose = true;
	vof->offset = 0;

	return (VFile_Handle) vif;
}