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
#include <dlfcn.h>
#endif

void *resolve_symb(void* dllinfo, const char* name) {
#ifdef Win32
	return GetProcAddress((HMODULE) dllinfo, name);
#else
	return dlsym(dllinfo, name);
#endif
}

int main(int argc, char *argv[]) {
// TODO: Take lib name from CMake?
// TODO: Use dlmopen, where available?
	const char* backendlib = getenv("RK_BACKEND_LIB");
	if (!backendlib) {
		fprintf(stderr, "Backend lib not specified!");
		exit(99);
	}
#ifdef Win32
	auto r_dllinfo = LoadLibraryA("R.dll");
	auto rkb_dllinfo = LoadLibraryA(backendlib);
#else
#	ifdef __APPLE__
	auto r_dllinfo = dlopen("libR.dylib", RTLD_NOW | RTLD_LOCAL);
#	else
	auto r_dllinfo = dlopen("libR.so", RTLD_NOW | RTLD_LOCAL);  // NOTE: RTLD_DEEPBIND causes undiagnosed runtime failure on Suse Tumbleweed around 05/24 (while it works, elsewhere)
#	endif
	auto rkb_dllinfo = dlopen(backendlib, RTLD_NOW | RTLD_LOCAL);
#endif
	if (!rkb_dllinfo) {
		fprintf(stderr, "Failure to open backend lib from %s", backendlib);
		exit(99);
	}
	if (!r_dllinfo) {
		fprintf(stderr, "Failure to open R lib");
		exit(99);
	}
	int (*do_main) (int, char**, void*, void* (*)(void*, const char*));
	do_main = (decltype(do_main)) resolve_symb(rkb_dllinfo, "do_main");
	if (!do_main) {
		fprintf(stderr, "Failure to resolve do_main()");
		exit(99);
	}
	return do_main(argc, argv, r_dllinfo, resolve_symb);
}
