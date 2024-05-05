/*
rkrapi - This file is part of RKWard (https://rkward.kde.org). Created: Wed May 01 2024
SPDX-FileCopyrightText: 2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkrapi.h"

#ifdef Q_OS_WIN
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include "../debug.h"

void RFn::init(const char* libname) {
#if defined RK_DLOPEN_LIBRSO
//#if defined(RTLD_DEEPBIND)  // causes hard fail on suse tumbleweed 05/2024
//	RK_DEBUG(RBACKEND, DL_DEBUG, "Now loading R lib, dynamically (deepbind)");
//	RFn::init(dlopen(RLIBNAME, RTLD_NOW | RTLD_DEEPBIND));
//#else
	RK_DEBUG(RBACKEND, DL_DEBUG, "Now loading R lib, dynamically (local)");
#if defined(Q_OS_WIN)
	auto dllinfo = LoadLibraryA(libname);
#else
	auto dllinfo = dlopen(libname, RTLD_NOW | RTLD_LOCAL);
#endif
	if (!dllinfo) {
		RK_DEBUG(RBACKEND, DL_ERROR, "Failure loading R lib from '%s'", libname);
	}
//#endif

	auto rfn = new RFn(); // we need a dummy object, even if we are only interested in the static members
	auto meta = rfn->metaObject();
	RK_DEBUG(RBACKEND, DL_DEBUG, "Loading %d symbols from R lib %p", meta->propertyCount() - meta->propertyOffset(), dllinfo);
	for (int i = meta->propertyOffset(); i < meta->propertyCount(); ++i) {
		auto prop = meta->property(i);
		auto name = prop.name();
#if defined(Q_OS_WIN)
		auto symb = GetProcAddress(dllinfo, name);
#else
		auto symb = dlsym(dllinfo, name);
#endif
		RK_DEBUG(RBACKEND, DL_DEBUG, "Lookup of symbol %s in %p: %p", name, dllinfo, symb);
		prop.write(rfn, QVariant::fromValue((void*) symb)); // NOTE: Qt refuses to write nullptr as value, but that's already the initial value of each member
	}
#else
	RK_DEBUG(RBACKEND, DL_DEBUG, "R lib already linked");
#endif
}
