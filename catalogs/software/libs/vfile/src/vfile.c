#include "core/core.h"
#include "dbg/assert.h"
#include "vfile/vfile.h"

#define VFILE_FUNC_HEADER  \
struct VFile_Interface_t* interface = (VFile_Interface_t*)handle; \
assert(interface); \
assert(interface->magic == InterfaceMagic);

EXTERN_C void VFile_Close(VFile_Handle handle) {
	VFILE_FUNC_HEADER
	interface->closeFunc(interface);
}

EXTERN_C void VFile_Flush(VFile_Handle handle) {
	VFILE_FUNC_HEADER
	interface->flushFunc(interface);
}
EXTERN_C size_t VFile_Read(VFile_Handle handle, void *buffer, size_t byteCount) {
	VFILE_FUNC_HEADER
	memset(buffer, 0, byteCount);
	return interface->readFunc(interface, buffer, byteCount);
}

EXTERN_C size_t VFile_Write(VFile_Handle handle, void const *buffer, size_t byteCount) {
	VFILE_FUNC_HEADER
	return interface->writeFunc(interface, buffer, byteCount);
}
EXTERN_C bool VFile_Seek(VFile_Handle handle, int64_t offset, enum VFile_SeekDir origin) {
	VFILE_FUNC_HEADER
	return interface->seekFunc(interface, offset, origin);
}
EXTERN_C int64_t VFile_Tell(VFile_Handle handle) {
	VFILE_FUNC_HEADER
	return interface->tellFunc(interface);
}
EXTERN_C size_t VFile_Size(VFile_Handle handle) {
	VFILE_FUNC_HEADER
	return interface->sizeFunc(interface);
}

EXTERN_C char const *VFile_GetName(VFile_Handle handle) {
	VFILE_FUNC_HEADER
	return interface->nameFunc(interface);
}

EXTERN_C bool VFile_IsEOF(VFile_Handle handle) {
	VFILE_FUNC_HEADER
	return interface->isEofFunc(interface);
}
EXTERN_C uint32_t VFile_GetType(VFile_Handle handle) {
	VFILE_FUNC_HEADER
	return interface->type;
}
EXTERN_C void* VFile_GetTypeSpecificData(VFile_Handle handle) {
	VFILE_FUNC_HEADER
	return (interface + 1);
}

#undef VFILE_FUNC_HEADER