/*
rkrbackend_dlopen - This file is part of RKWard (https://rkward.kde.org). Created: Mon May 06 2024
SPDX-FileCopyrightText: 2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <stdlib.h>
#include <stdio.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define Win32
#endif
#ifdef Win32
#include <windows.h>
#else
#include <unistd.h>
#include <string>
#include <dlfcn.h>
#endif

#include <filesystem>

void *resolve_symb(void* dllinfo, const char* name) {
#ifdef Win32
	return GetProcAddress((HMODULE) dllinfo, name);
#else
	return dlsym(dllinfo, name);
#endif
}

// See https://stackoverflow.com/questions/34813412/how-to-detect-if-building-with-address-sanitizer-when-building-with-gcc-4-8
#if defined(__has_feature)
#   if __has_feature(address_sanitizer) // for clang
#       define __SANITIZE_ADDRESS__
#   endif
#endif

auto loadlib(const char* name) {
#if defined(Win32)
	auto ret = LoadLibraryA(name);
#elif 0 && defined(LM_ID_NEWLM) && !defined(__SANITIZE_ADDRESS__)
	// NOTE / TODO: with this, we get Cstack use too close to the limit -> backend crash when loading library(tcltk)
	// This applies even when loading RK_BACKEND_LIB with dlmopen() and libR.so with plain dlopen(), or vice-versa
	auto ret = dlmopen(LM_ID_NEWLM, name, RTLD_NOW | RTLD_LOCAL);
#elif defined(RTLD_DEEPBIND) && !defined(__SANITIZE_ADDRESS__)
	auto ret = dlopen(name, RTLD_LAZY | RTLD_LOCAL | RTLD_DEEPBIND);
#else
	auto ret = dlopen(name, RTLD_NOW | RTLD_LOCAL);  // NOTE: RTLD_DEEPBIND or dlmopen causs runtime failure with address sanitization
#endif
	if (!ret) {
#ifdef Win32
		fprintf(stderr, "Failure to open lib %s\n", name);
#else
		fprintf(stderr, "Failure to open lib %s: %s\n", name, dlerror());
#endif
		exit(99);
	}
	return ret;
}

int main(int argc, char *argv[]) {
// TODO: Take lib name from CMake?

/** NOTE: For a description of the rationale for this involved loading procedure rkapi.h ! */
	if (argc > 10) {
		fprintf(stderr, "Too many args\n"); // and I'm lazy
		exit(99);
	}
#ifndef Win32
	// Prepend out own load path to what R CMD has set
	// Note that we're using a relative path that will only find something if starting from the correct dir
	const char RK_ADD_LDPATH[] = "RK_ADD_LDPATH";
	const char LD_LIBRARY_PATH[] = "LD_LIBRARY_PATH";
	const char* addldpath = getenv(RK_ADD_LDPATH);
	if (addldpath && addldpath[0]) {
		std::string curldpath = addldpath;
		char* c_curldpath = getenv(LD_LIBRARY_PATH);
		if (c_curldpath && c_curldpath[0]) {
			curldpath += ":";
			curldpath += c_curldpath;
		}
		setenv(LD_LIBRARY_PATH, curldpath.c_str(), 1);
		unsetenv(RK_ADD_LDPATH);
		char* const args[] = { argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], argv[8], argv[9], nullptr };
		execv(argv[0], args);
	}
#endif

	const char* backendlib = getenv("RK_BACKEND_LIB");
	if (!backendlib || !(backendlib[0])) {
		fprintf(stderr, "Backend lib not specified!\n");
		exit(99);
	}

#if defined(Win32)
	auto r_dllinfo = loadlib("R.dll");
#elif defined(__APPLE__)
	// libR.dylib is not always correctly linked against libRlapack.dylib, even where needed.
	// let's try to preload it into the global namespace
	dlopen("libRlapack.dylib", RTLD_LAZY | RTLD_GLOBAL);
	dlopen("libRblas.dylib", RTLD_LAZY | RTLD_GLOBAL);
	auto r_dllinfo = loadlib("libR.dylib");
#else
	auto r_dllinfo = loadlib("libR.so");
#endif

	char* c_rk_ld_cwd = getenv("RK_LD_CWD");
	auto cd = std::filesystem::current_path();
	if (c_rk_ld_cwd && c_rk_ld_cwd[0]) {
		std::filesystem::current_path(c_rk_ld_cwd);
	}
	auto rkb_dllinfo = loadlib(backendlib);
	std::filesystem::current_path(cd);

	int (*do_main) (int, char**, void*, void* (*)(void*, const char*));
	do_main = (decltype(do_main)) resolve_symb(rkb_dllinfo, "do_main");
	if (!do_main) {
#if defined(Win32)
		fprintf(stderr, "Failure to resolve do_main()\n");
#else
		fprintf(stderr, "Failure to resolve do_main(): %s\n", dlerror());
#endif
		exit(99);
	}
	return do_main(argc, argv, r_dllinfo, resolve_symb);
}
