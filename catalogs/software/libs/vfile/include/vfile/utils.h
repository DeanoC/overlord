#pragma once

#include "vfile/vfile.h"

EXTERN_C uint8_t VFile_ReadByte(VFile_Handle handle);
EXTERN_C char VFile_ReadChar(VFile_Handle handle);

EXTERN_C int8_t VFile_ReadInt8(VFile_Handle handle);
EXTERN_C int16_t VFile_ReadInt16(VFile_Handle handle);
EXTERN_C int32_t VFile_ReadInt32(VFile_Handle handle);
EXTERN_C int64_t VFile_ReadInt64(VFile_Handle handle);
EXTERN_C uint8_t VFile_ReadUInt8(VFile_Handle handle);
EXTERN_C uint16_t VFile_ReadUInt16(VFile_Handle handle);
EXTERN_C uint32_t VFile_ReadUInt32(VFile_Handle handle);
EXTERN_C uint64_t VFile_ReadUInt64(VFile_Handle handle);

EXTERN_C bool VFile_ReadBool(VFile_Handle handle);
EXTERN_C float VFile_ReadFloat(VFile_Handle handle);
EXTERN_C double VFile_ReadDouble(VFile_Handle handle);

//AL2O3_EXTERN_C Math_Vec2F VFile_ReadVec2F(VFile_Handle handle);
//AL2O3_EXTERN_C Math_Vec3F VFile_ReadVec3F(VFile_Handle handle);
//AL2O3_EXTERN_C Math_Vec3F VFile_ReadPackedVec3F(VFile_Handle handle, float maxAbsCoord);
//AL2O3_EXTERN_C Math_Vec4F VFile_ReadVec4F(VFile_Handle handle);

EXTERN_C size_t VFile_ReadString(VFile_Handle handle, char *buffer, size_t maxSize);
EXTERN_C void VFile_ReadFileID(VFile_Handle handle, char buffer[4]);
EXTERN_C size_t VFile_ReadLine(VFile_Handle handle, char *buffer, size_t maxSize);

#define VFILE_MAKE_ID(a,b,c,d) ((a) << 24U | (b) << 16U | (c) << 8U | (a) << 0U)
