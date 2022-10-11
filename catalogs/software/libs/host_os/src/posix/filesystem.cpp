#include "core/core.h"
#include "dbg/print.h"
#include "core/utf8.h"
#include "host_os/filesystem.hpp"
#include "memory/memory.h"
#include <unistd.h>       // getcwd
#include <errno.h>        // errno
#include <sys/stat.h>     // stat
#include <stdio.h>        // remove
#include <dirent.h>        // directory functions
#include <fcntl.h>
#include <sys/types.h>
#include <pwd.h>

#if defined(__APPLE__) || defined(__FreeBSD__)
	#include <copyfile.h>
#else
	#include <sys/sendfile.h>
#endif
// internal and platform path are the same on posix
EXTERN_C bool Os_IsNormalisedPath(char const *path) {
	return true;
}

EXTERN_C bool Os_GetNormalisedPathFromPlatformPath(char const *path, char *pathOut, size_t maxSize) {
	// just copy
	if (utf8size(path) >= maxSize) {
		return false;
	}
	utf8cpy(pathOut, path);
	return true;
}

EXTERN_C bool Os_GetPlatformPathFromNormalisedPath(char const *path, char *pathOut, size_t maxSize) {
	// just copy
	if (utf8size(path) >= maxSize) {
		return false;
	}
	utf8cpy(pathOut, path);
	return true;
}

// APPLE have tere own method called from objective-c
#if AL2O3_PLATFORM_OS == AL2O3_OS_GNULINUX
EXTERN_C bool Os_GetExePath(char *dirOut, int maxSize) {
	readlink("/proc/self/exe", dirOut, maxSize);
	return true;
}
#elif AL2O3_PLATFORM_OS == AL2O3_OS_FREEBSD
EXTERN_C bool Os_GetExePath(char *dirOut, int maxSize) {
    readlink("/proc/curproc/file", dirOut, maxSize);
    return true;
}
#endif

#if AL2O3_PLATFORM_OS != AL2O3_OS_OSX
EXTERN_C bool Os_GetUserDocumentsDir(char *dirOut, int maxSize) {
	const char *homedir;

	if ((homedir = getenv("HOME")) == NULL) {
	    homedir = getpwuid(getuid())->pw_dir;
	}
	if(utf8size(homedir) < maxSize) {
		utf8cpy(dirOut, homedir);
		return true;
	}
	return false;
}

EXTERN_C bool Os_GetAppPrefsDir(char const *org, char const *app, char *dirOut, int maxSize) {
  char homeDir[2048];
  if(Os_GetUserDocumentsDir(homeDir, sizeof(homeDir)) == false) {
  	return false;
  }
  if(utf8size(org) > 1024) {
  	return false;
  }
  if(utf8size(app) > 1024) {
  	return false;
  }

  char path[4096];
  sprintf(path, "%s\\%s\\%s", homeDir, org, app);

  if (utf8size(path) >= maxSize) { return false; }
  utf8cpy(dirOut, path);
  return true;
}

#endif

EXTERN_C size_t Os_GetLastModifiedTime(const char *fileName) {
	struct stat fileInfo;

	if (!stat(fileName, &fileInfo)) {
		return (size_t) fileInfo.st_mtime;
	} else {
		// return an impossible large mod time as the file doesn't exist
		return ~0;
	}
}

EXTERN_C bool Os_GetCurrentDir(char *dirOut, size_t maxSize) {
	char buffer[maxSize];
	if (getcwd(buffer, maxSize) == nullptr) {
		debug_print("ERROR: getcwd failed\n");
		return false;
	}
	if (!Os_GetPlatformPathFromNormalisedPath(buffer, dirOut, maxSize)) {
		debug_print("ERROR: Os_GetPlatformPathFromNormalisedPath failed");
		return false;
	}

	size_t len = utf8size(dirOut);
	if (dirOut[len-1] != '/') {
		if (len + 1 >= maxSize) {
			debug_print("ERROR: Os_GetCurrentDir maxSize too small");
			return false;
		}
		dirOut[len-1] = '/';
		dirOut[len] = 0;
	}
	return true;

}

EXTERN_C bool Os_SetCurrentDir(char const *path) {
	return chdir(path) == 0;
}

EXTERN_C bool Os_FileExists(char const *path) {
	struct stat st;
	int result = stat(path, &st);
	if (result != 0) {
		return false;
	}

	return !(st.st_mode & S_IFDIR);
}

EXTERN_C bool Os_DirExists(char const *path) {
	struct stat st;
	int result = stat(path, &st);
	if (result != 0) {
		return false;
	}

	return (st.st_mode & S_IFDIR);
}
EXTERN_C bool Os_FileCopy(char const *src, char const *dst) {
	char srcBuffer[2048];
	char dstBuffer[2048];

	if(utf8size(src) > 2048) {
		return false;
	}
	if(utf8size(dst) > 2048) {
		return false;
	}

	if (Os_IsNormalisedPath(src)) {
		utf8cpy(srcBuffer, src);
	} else {
		bool platformOk = Os_GetPlatformPathFromNormalisedPath(src, srcBuffer, sizeof(srcBuffer));
		if (platformOk == false) {
			return false;
		}
	}
	if (Os_IsNormalisedPath(dst)) {
		utf8cpy(dstBuffer, dst);
	} else {
		bool platformOk = Os_GetPlatformPathFromNormalisedPath(dst, dstBuffer, sizeof(dstBuffer));
		if (platformOk == false) {
			return false;
		}
	}

	// in kernel mode, zero overhead copy

	// from https://stackoverflow.com/questions/2180079/how-can-i-copy-a-file-on-unix-using-c
	int input, output;
	if ((input = open(srcBuffer, O_RDONLY)) == -1)
	{
		return -1;
	}
	if ((output = creat(dstBuffer, 0660)) == -1)
	{
		close(input);
		return -1;
	}

	//Here we use kernel-space copying for performance reasons
#if defined(__APPLE__) || defined(__FreeBSD__)
	//fcopyfile works on FreeBSD and OS X 10.5+
    int result = fcopyfile(input, output, 0, COPYFILE_ALL);
#else
	//sendfile will work with non-socket output (i.e. regular file) on Linux 2.6.33+
	off_t bytesCopied = 0;
	struct stat fileinfo = {0};
	fstat(input, &fileinfo);
	int result = sendfile(output, input, &bytesCopied, fileinfo.st_size);
#endif

	close(input);
	close(output);

	return result != -1;
}

EXTERN_C bool Os_FileDelete(char const *fileName) {
	char buffer[2048];

	if (Os_IsNormalisedPath(fileName)) {
		utf8cpy(buffer, fileName);
	} else {
		bool platformOk = Os_GetPlatformPathFromNormalisedPath(fileName, buffer, sizeof(buffer));
		if (platformOk == false) {
			return false;
		}
	}

	return remove(buffer) == 0;
}

EXTERN_C bool Os_DirCreate(char const *pathName) {
	using namespace Os::FileSystem;

	// Create each of the parents if necessary
	std::string parentPath = GetParentPath(pathName);
	if ((uint32_t) parentPath.size() > 1 && !DirExists(parentPath)) {
		if (!Os_DirCreate(parentPath.c_str())) {
			return false;
		}
	}

	bool success = mkdir(GetPlatformPathFromNormalisedPath(pathName).c_str(), S_IRWXU) == 0 || errno == EEXIST;

	return success;
}

EXTERN_C int Os_SystemRun(char const *fileName, int argc, const char **argv) {
	std::string fixedFileName = Os::FileSystem::GetPlatformPathFromNormalisedPath(fileName);

#if defined(__linux__)
	std::vector<const char*> argPtrs;
	std::string              cmd(fixedFileName.c_str());
	char                         space = ' ';
	cmd.append(&space, &space + 1);
	for (unsigned i = 0; i < (unsigned) argc; ++i) {
		std::string arg(argv[i]);
		cmd.append(arg);
	}

	int res = system(cmd.c_str());
	return res;
#else
	pid_t pid = fork();
	if (pid == 0) {
		// child processs
		std::vector<const char *> argPtrs;
		argPtrs.push_back(fixedFileName.c_str());
		for (unsigned i = 0; i < (unsigned) argc; ++i) {
			argPtrs.push_back(argv[i]);
		}
		argPtrs.push_back(NULL);

		execv(argPtrs[0], (char **) &argPtrs[0]);
		// only get here if execvp failed, so log the error
		// and kill the child!
		perror("execv failed");
		exit(-1);
	} else if (pid > 0) {
		// parent process
		int exitCode = EINTR;
		while (exitCode == EINTR) {
			wait(&exitCode);
		}
		return exitCode;
	} else {
		return -1;
	}
#endif
}

struct Os_DirectoryEnumerator {
		char path[2048];
		DIR *dir;
		Memory_Allocator* allocator;

		Os_DirectoryEnumeratorItem lastItem;
		bool cancelled;
};

EXTERN_C Os_DirectoryEnumeratorHandle Os_DirectoryEnumeratorCreate(char const *cpath, Memory_Allocator* allocator) {
	assert(cpath);

	auto enumerator = (Os_DirectoryEnumerator *) MALLOC(allocator, sizeof(Os_DirectoryEnumerator));
	if (enumerator == nullptr)
		return nullptr;

	Os_GetPlatformPathFromNormalisedPath(cpath, enumerator->path, sizeof(enumerator->path));

	enumerator->allocator = allocator;
	enumerator->dir = nullptr;
	enumerator->cancelled = false;
	return (Os_DirectoryEnumeratorHandle) enumerator;
}

EXTERN_C void Os_DirectoryEnumeratorDestroy(Os_DirectoryEnumeratorHandle handle) {
	assert(handle != nullptr);
	auto enumerator = (Os_DirectoryEnumerator *) handle;
	if (enumerator->dir != nullptr)
		closedir(enumerator->dir);

	MFREE(enumerator->allocator, enumerator);
}

EXTERN_C void Os_DirectoryEnumeratorAsyncStart(Os_DirectoryEnumeratorHandle handle,
Os_DirectoryEnumeratorAsyncFunc func, void *userData) {
	debug_print("WARNING: Os_DirectoryEnumeratorAsyncStart isn't async on posix yet, will be a busy sync for now");

	while (auto item = Os_DirectoryEnumeratorSyncNext(handle)) {
		func(handle, userData, item);
	}
}

EXTERN_C Os_DirectoryEnumeratorItem const *Os_DirectoryEnumeratorSyncNext(Os_DirectoryEnumeratorHandle handle) {
	assert(handle != nullptr);
	auto enumerator = (Os_DirectoryEnumerator *) handle;
	if (enumerator->cancelled) return nullptr;

	if (enumerator->dir == nullptr) {
		enumerator->dir = opendir(enumerator->path);
		if (enumerator->dir == nullptr) {
			return nullptr;
		}
	}

	dirent *entry = readdir(enumerator->dir);
	if (entry == nullptr) return nullptr;
	// skip . and ..
	if (utf8ncmp(entry->d_name, ".", 1) == 0) {
		return Os_DirectoryEnumeratorSyncNext(handle);
	}
	if (utf8ncmp(entry->d_name, "..", 2) == 0) {
		return Os_DirectoryEnumeratorSyncNext(handle);
	}

	enumerator->lastItem.filename = entry->d_name;
	enumerator->lastItem.directory = entry->d_type == DT_DIR;

	return &enumerator->lastItem;
}

EXTERN_C bool Os_DirectoryEnumeratorCancel(Os_DirectoryEnumeratorHandle handle) {
	assert(handle != nullptr);
	auto enumerator = (Os_DirectoryEnumerator *) handle;
	enumerator->cancelled = true;
	return true;
}
EXTERN_C bool Os_DirectoryEnumeratorStallForAll(Os_DirectoryEnumeratorHandle handle) {
	// TODO no async means this doesn't do anything
	return true;
}
