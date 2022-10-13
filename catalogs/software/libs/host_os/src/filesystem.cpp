#include "core/core.h"
#include "dbg/assert.h"
#include "core/utf8.h"
#include "host_os/filesystem.h"

EXTERN_C bool Os_SplitPath(utf8_int8_t const * path, size_t *fileName, size_t *extension) {
  assert(path != nullptr);
  assert(Os_IsNormalisedPath(path));

  bool result = false;
  if (utf8len(path) == 0) { return result; }

  if (fileName != nullptr) {
		utf8_int8_t const * const lastSlash = utf8rchr(path, '/');
		if(lastSlash != nullptr) *fileName = lastSlash - path;
		else *fileName = 0;
		result = true;
  }

  if (extension != nullptr) {
		utf8_int8_t const * const lastDot = utf8rchr(path, '.');
		if(lastDot != nullptr) *extension = lastDot - path;
		else *extension = 0;
		result = true;
  }

  return result;
}

EXTERN_C bool Os_ReplaceExtension(utf8_int8_t const * path,
									utf8_int8_t const * newExtension,
									utf8_int8_t * dirOut,
									size_t maxSize) {
  assert(path);
  assert(newExtension);
  assert(dirOut);

  utf8_int8_t const * const lastDot = utf8rchr(path, '.');
  if(lastDot == nullptr) {
    utf8_int8_t * tmpExt = (utf8_int8_t *) STACK_ALLOC(utf8size(newExtension + 1));
    tmpExt[0] = '.';
    memcpy(tmpExt+1, newExtension, utf8size(newExtension));
    utf8ncpy(dirOut, path, maxSize);
    utf8ncat(dirOut, tmpExt, maxSize);

  } else {
    utf8ncpy(dirOut, path, (lastDot - path) + 1);
    utf8ncat(dirOut, newExtension, maxSize);
  }

  return true;
}

EXTERN_C bool Os_GetParentPath(utf8_int8_t const * path, utf8_int8_t *dirOut, size_t maxSize) {
	assert(path);
	assert(dirOut);

	utf8ncpy(dirOut, path, maxSize);

	size_t const size = utf8size(dirOut);
	if(size > 0 ) {
		// remove final '/' if it exists
		if (dirOut[size] == '/') {
			dirOut[size] = '0';
		}
		utf8_int8_t *const lastSlash = utf8rchr(dirOut, '/');
		if (lastSlash != nullptr) {
			*lastSlash = '\0';
		}
	}

	return true;
}
