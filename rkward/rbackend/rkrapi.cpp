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

void RFn::init(void* libr_dll_handle, void* (*dlsym_fun)(void*, const char*)) {
#if defined RK_DLOPEN_LIBRSO
	auto rfn = new RFn(); // we need a dummy object, even if we are only interested in the static members
	auto meta = rfn->metaObject();
	RK_DEBUG(RBACKEND, DL_DEBUG, "Loading %d symbols from R lib %p", meta->propertyCount() - meta->propertyOffset(), libr_dll_handle);
	for (int i = meta->propertyOffset(); i < meta->propertyCount(); ++i) {
		auto prop = meta->property(i);
		auto name = prop.name();
		auto symb = dlsym_fun(libr_dll_handle, name);
		RK_DEBUG(RBACKEND, DL_DEBUG, "Lookup of symbol %s in %p: %p", name, libr_dll_handle, symb);
		prop.write(rfn, QVariant::fromValue((void*) symb)); // NOTE: Qt refuses to write nullptr as value, but that's already the initial value of each member
	}
#else
	RK_DEBUG(RBACKEND, DL_DEBUG, "R lib already linked");
#endif
}
