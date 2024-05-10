/*
rkeditobjectagent - This file is part of RKWard (https://rkward.kde.org). Created: Fri Feb 16 2007
SPDX-FileCopyrightText: 2007-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rkeditobjectagent.h"

#include <KLocalizedString>
#include <kmessagebox.h>

#include "../core/robjectlist.h"
#include "../rbackend/rkrinterface.h"
#include "../rkward.h"
#include "../windows/rkworkplace.h"

#include "../debug.h"

RKEditObjectAgent::RKEditObjectAgent (const QStringList &_object_names, RCommandChain *chain) : object_names(_object_names) {
	RK_TRACE (APP);

	// first issue an empty command to trigger an update of the object list
	RInterface::issueCommand (new RCommand (QString (), RCommand::EmptyCommand | RCommand::ObjectListUpdate), chain);

	// now add another empty command to find out, when the update has completed
	RInterface::whenAllFinished(this, [this]() {
		for (QStringList::const_iterator it = object_names.constBegin (); it != object_names.constEnd (); ++it) {
			QString object_name = *it;
			RObject *obj = RObjectList::getObjectList ()->findObject (object_name);
			if (!(obj && RKWorkplace::mainWorkplace()->editObject (obj))) {
				KMessageBox::information(nullptr, i18n("The object '%1', could not be opened for editing. Either it does not exist, or RKWard does not support editing this type of object, yet.", object_name), i18n("Cannot edit '%1'", object_name));
			}
		}

		// we're done
		deleteLater ();
	}, chain);
}

RKEditObjectAgent::~RKEditObjectAgent () {
	RK_TRACE (APP);
}
