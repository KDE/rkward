/***************************************************************************
                          rkimportdialog  -  description
                             -------------------
    begin                : Tue Jan 30 2007
    copyright            : (C) 2007 by Thomas Friedrichsmeier
    email                : tfry@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "rkimportdialog.h"

#include <kmessagebox.h>
#include <kfilefiltercombo.h>
#include <klocale.h>

#include <qcombobox.h>
#include <qlabel.h>

#include "../plugin/rkcomponentmap.h"
#include "../plugin/rkcomponentcontext.h"

#include "../debug.h"

RKImportDialogFormatSelector::RKImportDialogFormatSelector () : KHBox () {
	RK_TRACE (DIALOGS);

	new QLabel (i18n ("File format: "), this);
	combo = new QComboBox (this);
}

RKImportDialog::RKImportDialog (const QString &context_id, QWidget *parent) : KFileDialog (KUrl (), QString (), parent, format_selector=new RKImportDialogFormatSelector ()) {
	RK_TRACE (DIALOGS);

	setModal (false);

	context = RKComponentMap::getContext (context_id);
	if (!context) {
		KMessageBox::sorry (this, i18n ("No plugins defined for context '%1'", context_id));
		return;
	}

	component_ids = context->components ();
	QString formats = "*|" + i18n ("All Files") + " (*)\n";
	int format_count = 0;
	for (QStringList::const_iterator it = component_ids.constBegin (); it != component_ids.constEnd (); ++it) {
		if (format_count++) formats.append ('\n');

		RKComponentHandle *handle = RKComponentMap::getComponentHandle (*it);
		if (!handle) {
			RK_ASSERT (false);
			continue;
		}

		QString filter = handle->getAttributeValue ("format");
		QString label = handle->getAttributeLabel ("format");

		QString elabel = label;
		elabel.replace ('/', "\\/");
		elabel.replace ('|', "\\|");
		formats.append (filter + '|' + elabel + " (" + filter + ')');

		format_labels.append (label);
		filters.append (filter);
	}

	// file format selection box
	format_selector->combo->insertItems (0, format_labels);

	// initialize
	setMode (KFile::File | KFile::ExistingOnly | KFile::LocalOnly);
	setFilter (formats);
	connect (this, SIGNAL (filterChanged (const QString&)), this, SLOT (filterWasChanged (const QString&)));
	filterWasChanged (QString ());
	show ();
}

RKImportDialog::~RKImportDialog () {
	RK_TRACE (DIALOGS);
}

void RKImportDialog::filterWasChanged (const QString &) {
	RK_TRACE (DIALOGS);

	int index = filters.indexOf (filterWidget ()->currentFilter ());

	if (index < 0) {		// All files
		format_selector->combo->setEnabled (true);
	} else {
		format_selector->combo->setEnabled (false);
		format_selector->combo->setCurrentIndex (index);
	}
}

void RKImportDialog::accept () {
	RK_TRACE (DIALOGS);

	KFileDialog::accept ();

	int index = format_selector->combo->currentIndex ();
	QString cid = component_ids[index];
	RKComponentHandle *handle = RKComponentMap::getComponentHandle (cid);
	RKContextHandler *chandler = context->makeContextHandler (this, false);

	if (!(handle && chandler)) {
		RK_ASSERT (false);
	} else {
		RKComponentPropertyBase *filename = new RKComponentPropertyBase (chandler, false);
		filename->setValue (selectedFile ());
		chandler->addChild ("filename", filename);

		chandler->invokeComponent (handle);
	}

	deleteLater ();
}

void RKImportDialog::reject () {
	RK_TRACE (DIALOGS);

	KFileDialog::reject ();
	deleteLater ();
}

#include "rkimportdialog.moc"
