#pragma once
#include "core/core.h"
#include "dbg/assert.h"
#include "host_os/filesystem.h"
#include <string>
#include <vector>

namespace Os {
		namespace FileSystem {

				typedef void (*FileDialogCallbackFn)(std::string url, void *userData);

				inline bool IsNormalisedPath(std::string const& path) {
					return Os_IsNormalisedPath(path.c_str());
				}

				inline std::string GetNormalisedPathFromPlatformPath(std::string const& path) {
					char tmp[2048];
					if (Os_GetNormalisedPathFromPlatformPath(path.c_str(), tmp, 2048)) {
						return std::string(tmp);
					} else {
						return std::string();
					}
				}

				inline std::string GetPlatformPathFromNormalisedPath(std::string const& path) {
					char tmp[2048];
					if (Os_GetPlatformPathFromNormalisedPath(path.c_str(), tmp, sizeof(tmp))) {
						return std::string(tmp);
					} else {
						return std::string();
					}
				}

				inline bool IsAbsolutePath(std::string const& path) {
					return Os_IsAbsolutePath(path.c_str());
				}

				bool SplitPath(std::string const& fullPath, std::string_view& fileName,
				               std::string_view& extension);
				inline bool SplitPath(std::string const& fullPath, std::string& fileName,
				                      std::string& extension) {
					std::string_view fn, ext;
					bool result = SplitPath(fullPath, fn, ext);
					fileName = fn;
					extension = ext;
					return result;
				}

				inline std::string ReplaceExtension(std::string const& path,
				                                        std::string const& newExtension) {
					char tmp[2048];
					bool okayReplaceExt = Os_ReplaceExtension(path.c_str(), newExtension.c_str(), tmp, sizeof(2048));
					if (okayReplaceExt) { return std::string(tmp); }
					else { return {}; }
				}

				inline std::string GetParentPath(std::string const& path) {
					char tmp[2048];
					bool okayParentPath = Os_GetParentPath(path.c_str(), tmp, sizeof(2048));
					if (okayParentPath) { return std::string(tmp); }
					else { return {}; }
				}

				inline std::string GetCurrentDir() {
					char tmp[2048];
					if (Os_GetCurrentDir(tmp, sizeof(tmp))) {
						return std::string(tmp);
					} else {
						return std::string();
					}
				}

				inline bool SetCurrentDir(std::string const& path) {
					return Os_SetCurrentDir(path.c_str());
				}

				inline bool FileExists(std::string const& path) {
					return Os_FileExists(path.c_str());
				}

				inline bool DirExists(std::string const& path) {
					return Os_DirExists(path.c_str());
				}

				inline bool FileCopy(std::string const& src, std::string const& dst) {
					return Os_FileCopy(src.c_str(), dst.c_str());
				}

				inline bool FileDelete(std::string const& src) {
					return Os_FileDelete(src.c_str());
				}

				inline bool DirCreate(std::string const& dir) {
					return Os_DirCreate(dir.c_str());
				}

				inline std::string GetFileName(std::string const& path) {
					std::string_view fileName;
					std::string_view extension;
					bool splitOk = FileSystem::SplitPath(path, fileName, extension);
					if (splitOk) { return std::string(fileName); }
					else { return {}; }
				}

				inline std::string GetExtension(std::string const& path) {
					std::string_view fileName;
					std::string_view extension;
					bool splitOk = FileSystem::SplitPath(path, fileName, extension);
					if (splitOk) { return std::string(extension); }
					else { return {}; }
				}

				inline std::string GetExePath() {
					char tmp[2048];
					if (Os_GetExePath(tmp, sizeof(tmp))) {
						return std::string(tmp);
					} else { return {}; }
				}

				inline std::string GetUserDocumentsDir() {
					char tmp[2048];
					if (Os_GetUserDocumentsDir(tmp, sizeof(tmp))) {
						return std::string(tmp);
					} else { return {}; }
				}

				inline std::string GetAppPrefsDir(std::string const& org, std::string const& app) {
					char tmp[2048];
					if (Os_GetAppPrefsDir(org.c_str(), app.c_str(), tmp, sizeof(tmp))) {
						return std::string(tmp);
					} else { return {}; }
				}

				inline int SystemRun(std::string const& fileName, std::vector<std::string> const& args) {
					char const* cargs[100];
					assert(args.size() < 100);
					for (auto i = 0u; i < args.size(); ++i) {
						cargs[i] = args[i].c_str();
					}
					return Os_SystemRun(fileName.c_str(), (int)args.size(), cargs);
				}

				inline size_t GetLastModifiedTime(std::string const& fileName) {
					return Os_GetLastModifiedTime(fileName.c_str());
				}

		} // namespace FileSystem
} // end namespace Os
