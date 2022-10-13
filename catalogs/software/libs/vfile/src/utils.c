#include "core/core.h"
#include "vfile/utils.h"

EXTERN_C uint8_t VFile_ReadByte(VFile_Handle handle) {
  uint8_t ret;
  VFile_Read(handle, &ret, sizeof(ret));
  return ret;
}

EXTERN_C char VFile_ReadChar(VFile_Handle handle) {
  char ret;
  VFile_Read(handle, &ret, sizeof(ret));
  return ret;
}

EXTERN_C int8_t VFile_ReadInt8(VFile_Handle handle) {
  int8_t ret;
  VFile_Read(handle, &ret, sizeof(ret));
  return ret;
}

EXTERN_C int16_t VFile_ReadInt16(VFile_Handle handle) {
  int16_t ret;
  VFile_Read(handle, &ret, sizeof(ret));
  return ret;
}

EXTERN_C int32_t VFile_ReadInt32(VFile_Handle handle) {
  int32_t ret;
  VFile_Read(handle, &ret, sizeof(ret));
  return ret;
}

EXTERN_C int64_t VFile_ReadInt64(VFile_Handle handle) {
  int64_t ret;
  VFile_Read(handle, &ret, sizeof(ret));
  return ret;
}

EXTERN_C uint8_t VFile_ReadUInt8(VFile_Handle handle) {
  uint8_t ret;
  VFile_Read(handle, &ret, sizeof(ret));
  return ret;
}

EXTERN_C uint16_t VFile_ReadUInt16(VFile_Handle handle) {
  uint16_t ret;
  VFile_Read(handle, &ret, sizeof(ret));
  return ret;
}

EXTERN_C uint32_t VFile_ReadUInt32(VFile_Handle handle) {
  uint32_t ret;
  VFile_Read(handle, &ret, sizeof(ret));
  return ret;
}

EXTERN_C uint64_t VFile_ReadUInt64(VFile_Handle handle) {
  uint64_t ret;
  VFile_Read(handle, &ret, sizeof(ret));
  return ret;
}

EXTERN_C bool VFile_ReadBool(VFile_Handle handle) {
  return VFile_ReadByte(handle) != 0;
}

EXTERN_C float VFile_ReadFloat(VFile_Handle handle) {
  float ret;
	VFile_Read(handle, &ret, sizeof(ret));
	return ret;
}

EXTERN_C double VFile_ReadDouble(VFile_Handle handle) {
	double ret;
	VFile_Read(handle, &ret, sizeof(ret));
	return ret;
}
/*
EXTERN_C Math_Vec2F VFile_ReadVec2F(VFile_Handle handle) {
	Math_Vec2F data;
	VFile_Read(handle, &data, sizeof(float) * 2);
	return data;
}

EXTERN_C Math_Vec3F VFile_ReadVec3F(VFile_Handle handle) {
	Math_Vec3F data;
	VFile_Read(handle, &data, sizeof(float) * 3);
	return data;
}

EXTERN_C Math_Vec3F VFile_ReadPackedVec3F(VFile_Handle handle, float maxAbsCoord) {
	float invV = maxAbsCoord / 32767.0f;
	int16_t coords[3];
	VFile_Read(handle, &coords, sizeof(int16_t) * 3);

	Math_Vec3F ret = {
			coords[0] * invV, coords[1] * invV, coords[2] * invV
	};
	return ret;
}

EXTERN_C Math_Vec4F VFile_ReadVec4F(VFile_Handle handle) {
	Math_Vec4F data;
	VFile_Read(handle, &data, sizeof(float) * 4);
	return data;
}*/

EXTERN_C size_t VFile_ReadString(VFile_Handle handle, char *buffer, size_t maxSize) {
	size_t pos = 0;
	while (!VFile_IsEOF(handle)) {
		if (pos >= maxSize) {
			return pos;
		}

		char c = VFile_ReadChar(handle);
    buffer[pos++] = c;
    if (c == 0) {
      return pos;
    }
  }
  return pos;
}

EXTERN_C void VFile_ReadFileID(VFile_Handle handle, char buffer[4]) {
  VFile_Read(handle, buffer, sizeof(char) * 4);
}

EXTERN_C size_t VFile_ReadLine(VFile_Handle handle, char *buffer, size_t maxSize) {
  size_t pos = 0;
  while (!VFile_IsEOF(handle)) {
    if (pos >= maxSize) { return pos; }
    char c = VFile_ReadChar(handle);

    if (c == 0 || c == 10) {
      return pos;
    } else if (c == 13) {
      // Peek next char to see if it's 10, and skip it too
      if (!VFile_IsEOF(handle)) {
        char n = VFile_ReadChar(handle);
        if (n != 10) {
          VFile_Seek(handle, -1, VFile_SD_Current);
        }
      }
      return pos;
    } else {
      buffer[pos++] = c;
    }
  }
  return pos;
}