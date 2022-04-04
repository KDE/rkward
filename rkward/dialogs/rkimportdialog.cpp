/*
rkimportdialog - This file is part of RKWard (https://rkward.kde.org). Created: Tue Jan 30 2007
SPDX-FileCopyrightText: 2007-2018 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkimportdialog.h"

#include <kmessagebox.h>
#include <KLocalizedString>

#include <qcombobox.h>
#include <qlabel.h>

#include "../plugin/rkcomponentmap.h"
#include "../plugin/rkcomponentcontext.h"

#include "../debug.h"

RKImportDialog::RKImportDialog (const QString &context_id, QWidget *parent) : QFileDialog (parent) {
	RK_TRACE (DIALOGS);

	setModal (false);

	context = RKComponentMap::getContext (context_id);
	if (!context) {
		KMessageBox::sorry (this, i18n ("No plugins defined for context '%1'", context_id));
		return;
	}

	component_ids = context->components ();
	for (QStringList::const_iterator it = component_ids.constBegin (); it != component_ids.constEnd (); ++it) {
		RKComponentHandle *handle = RKComponentMap::getComponentHandle (*it);
		if (!handle) {
			RK_ASSERT (false);
			continue;
		}

		QString filter = handle->getAttributeValue ("format");
		QString label = handle->getAttributeLabel ("format");

		QString elabel = label;
		elabel.replace ('(', "[");
		elabel.replace (')', "]");
		filters.append (elabel + " [" + filter + "] (" + filter + ')');
	}

	// initialize
	setFileMode (QFileDialog::ExistingFile);
	setNameFilters (filters);
	show ();
}

RKImportDialog::~RKImportDialog () {
	RK_TRACE (DIALOGS);
}

void RKImportDialog::accept () {
	RK_TRACE (DIALOGS);

	int index = filters.indexOf (selectedNameFilter ());
	QString cid = component_ids.value (index);
	RKComponentHandle *handle = RKComponentMap::getComponentHandle (cid);
	RKContextHandler *chandler = context->makeContextHandler (this, false);

	if (!(handle && chandler)) {
		RK_ASSERT (false);
	} else {
		RKComponentPropertyBase *filename = new RKComponentPropertyBase (chandler, false);
		filename->setValue (selectedFiles ().value (0));
		chandler->addChild ("filename", filename);

		chandler->invokeComponent (handle);
	}

	QFileDialog::accept ();
	deleteLater ();
}

void RKImportDialog::reject () {
	RK_TRACE (DIALOGS);

	QFileDialog::reject ();
	deleteLater ();
}

