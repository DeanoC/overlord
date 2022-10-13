// Full license at end of file
// Summary: Apache - original from The-Forge modified by DeanoC

#pragma once

#include "core/core.h"
#include "memory/memory.h"

static const size_t FS_npos = (size_t) (-1);

// is the provided path a platform or the a normalised path(they can be the same)
// internal paths are normalized to use forward slash '/' as directory seperator
// platform paths are slashes in the platform preferred direction
EXTERN_C bool Os_IsNormalisedPath(char const *pathName);
EXTERN_C bool Os_GetNormalisedPathFromPlatformPath(char const *pathName, char *dirOut, size_t maxSize);
EXTERN_C bool Os_GetPlatformPathFromNormalisedPath(char const *pathName, char *dirOut, size_t maxSize);

EXTERN_C bool Os_IsAbsolutePath(char const *path);
EXTERN_C bool Os_SplitPath(char const *path, size_t *fileName, size_t *extension);
EXTERN_C bool Os_ReplaceExtension(char const *path, char const *newExtension, char *dirOut, size_t maxSize);
EXTERN_C bool Os_GetParentPath(char const *path, char *dirOut, size_t maxSize);

EXTERN_C bool Os_GetCurrentDir(char *dirOut, size_t maxSize);
EXTERN_C bool Os_SetCurrentDir(char const *dir);

EXTERN_C bool Os_FileExists(char const *fileName);
EXTERN_C bool Os_DirExists(char const *pathName);

EXTERN_C bool Os_FileCopy(char const *src, char const *dst);
EXTERN_C bool Os_FileDelete(char const *fileName);
EXTERN_C bool Os_DirCreate(char const *pathName);
EXTERN_C int Os_SystemRun(char const *fileName, int argc, const char **argv);


EXTERN_C bool Os_GetExePath(char *dirOut, int maxSize);
EXTERN_C bool Os_GetUserDocumentsDir(char *dirOut, int maxSize);
EXTERN_C bool Os_GetAppPrefsDir(char const *org, char const *app, char *dirOut, int maxSize);

EXTERN_C size_t Os_GetLastModifiedTime(char const *fileName);

typedef struct  {
	char const* filename;
	bool directory;
} Os_DirectoryEnumeratorItem;

typedef struct Os_DirectoryEnumerator *Os_DirectoryEnumeratorHandle;
typedef void (*Os_DirectoryEnumeratorAsyncFunc)(Os_DirectoryEnumeratorHandle handle, void* userData, Os_DirectoryEnumeratorItem const * item);

EXTERN_C Os_DirectoryEnumeratorHandle Os_DirectoryEnumeratorCreate(char const* path, Memory_Allocator* allocator);
EXTERN_C void Os_DirectoryEnumeratorDestroy(Os_DirectoryEnumeratorHandle handle);

EXTERN_C void Os_DirectoryEnumeratorAsyncStart(Os_DirectoryEnumeratorHandle handle, Os_DirectoryEnumeratorAsyncFunc func, void* userData);
EXTERN_C bool Os_DirectoryEnumeratorAsyncStall(Os_DirectoryEnumeratorHandle handle);
EXTERN_C bool Os_DirectoryEnumeratorCancel(Os_DirectoryEnumeratorHandle handle);

EXTERN_C Os_DirectoryEnumeratorItem const* Os_DirectoryEnumeratorSyncNext(Os_DirectoryEnumeratorHandle handle);

/*
 * Copyright (c) 2018-2019 Confetti Interactive Inc.
 *
 * This file is part of The-Forge
 * (see https://github.com/ConfettiFX/The-Forge).
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
*/