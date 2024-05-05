/*
rkrapi - This file is part of RKWard (https://rkward.kde.org). Created: Wed May 01 2024
SPDX-FileCopyrightText: 2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkrapi.h"

#include <dlfcn.h>

#include "../debug.h"

void RFn::init(void *dllinfo) {
	// TODO: This is just proof/test of concept code!
	auto rfn = new RFn(); // we need a dummy object, even if we are only interested in the static members
	auto meta = rfn->metaObject();
	RK_DEBUG(RBACKEND, DL_DEBUG, "Loading %d symbols from R lib %p", meta->propertyCount() - meta->propertyOffset(), dllinfo);
	for (int i = meta->propertyOffset(); i < meta->propertyCount(); ++i) {
		auto prop = meta->property(i);
		auto name = prop.name();
		auto symb = dlsym(dllinfo, name);
		RK_DEBUG(RBACKEND, DL_DEBUG, "Lookup of symbol %s in %p: %p", name, dllinfo, symb);
		prop.write(rfn, QVariant::fromValue(symb)); // NOTE: Qt refuses to write nullptr as value, but that's already the initial value of each member
	}
}
