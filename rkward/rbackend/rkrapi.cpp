/*
rkrapi - This file is part of RKWard (https://rkward.kde.org). Created: Wed May 01 2024
SPDX-FileCopyrightText: 2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkrapi.h"

void RFn::init(void *dllinfo) {
	// TODO: This is just proof/test of concept code!
	auto dummy_rfn = new RFn();
	auto meta = dummy_rfn->metaObject();
	for (int i = 0; i < meta->propertyCount(); ++i) {
		auto prop = meta->property(i+meta->propertyOffset());
		auto name = prop.name();
		//RFn::Rf_warning("%d %s %d %d", i, name, prop.isReadable(), prop.isWritable());
		//RFn::Rf_warning("%d", prop.write(dummy_rfn, prop.read(dummy_rfn))); //dummy_rfn->setProperty(name, QVariant::fromValue(nullptr))); // NOTE: Qt refuses to write nullptr as value!
	}
	dummy_rfn->setProperty("Rf_warning", QVariant::fromValue((void*) RFn::Rf_error));
	RFn::Rf_warning("warning as error");
}
