/*
rkrbackend_dlopen - This file is part of RKWard (https://rkward.kde.org). Created: Mon May 06 2024
SPDX-FileCopyrightText: 2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifdef Q_OS_WIN
#include <windows.h>
#else
#include <dlfcn.h>
#endif

void *resolve_symb(void* dllinfo, const char* name) {
#if defined(Q_OS_WIN)
	retrun GetProcAddress(dllinfo, name);
#else
	return dlsym(dllinfo, name);
#endif
}

int main(int argc, char *argv[]) {
// TODO: Debugging!
// TODO: Take lib name from CMake?
#ifdef Q_OS_WIN
	auto r_dllinfo = LoadLibraryA("R.dll");
	auto rkb_dllinfo = LoadLibraryA("rkward.rbackend.lib.dll");
#else
	auto r_dllinfo = dlopen("libR.so", RTLD_NOW | RTLD_LOCAL | RTLD_DEEPBIND);
	auto rkb_dllinfo = dlopen("librkward.rbackend.lib.so", RTLD_NOW | RTLD_LOCAL | RTLD_DEEPBIND);
#endif
	int (*do_main) (int, char**, void*, void* (*)(void*, const char*));
	do_main = (decltype(do_main)) resolve_symb(rkb_dllinfo, "do_main");
	return do_main(argc, argv, r_dllinfo, resolve_symb);
}
