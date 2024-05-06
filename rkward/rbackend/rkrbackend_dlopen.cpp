/*
rkrbackend_dlopen - This file is part of RKWard (https://rkward.kde.org). Created: Mon May 06 2024
SPDX-FileCopyrightText: 2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

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
// TODO: Debugging!
// TODO: Take lib name from CMake?
#ifdef Win32
	auto r_dllinfo = LoadLibraryA("R.dll");
	auto rkb_dllinfo = LoadLibraryA("rkward.rbackend.lib.dll");
#else
	auto r_dllinfo = dlopen("libR.so", RTLD_NOW | RTLD_LOCAL);  // NOTE: RTLD_DEEPBIND causes undiagnosed runtime failure on Suse Tumbleweed around 05/24 (while it works, elsewhere)
	auto rkb_dllinfo = dlopen("librkward.rbackend.lib.so", RTLD_NOW | RTLD_LOCAL);
#endif
	int (*do_main) (int, char**, void*, void* (*)(void*, const char*));
	do_main = (decltype(do_main)) resolve_symb(rkb_dllinfo, "do_main");
	return do_main(argc, argv, r_dllinfo, resolve_symb);
}
