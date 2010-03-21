/***************************************************************************
                          rkselectlistdialog  -  description
                             -------------------
    begin                : Thu Mar 18 2010
    copyright            : (C) 2010 by Thomas Friedrichsmeier
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

#include "rkselectlistdialog.h"

#include <QListWidget>
#include <QLabel>

#include <klocale.h>
#include <kvbox.h>

#include "../debug.h"

RKSelectListDialog::RKSelectListDialog (QWidget *parent, const QString &caption, const QStringList& choices, const QStringList& preselected, bool multiple) : KDialog (parent) {
	RK_TRACE (DIALOGS);

	setModal (true);
	setCaption (caption);
	setButtons (KDialog::Ok | KDialog::Cancel);

	KVBox *page = new KVBox ();
	setMainWidget (page);

	if (multiple) new QLabel (i18n ("<b>Select one or more:</b>"), page);
	else new QLabel (i18n ("<b>Select one:</b>"), page);
	new QLabel (caption, page);

	input = new QListWidget (page);
	input->addItems (choices);
	if (multiple) input->setSelectionMode (QAbstractItemView::MultiSelection);
	else input->setSelectionMode (QAbstractItemView::SingleSelection);
	for (int i = 0; i < preselected.length (); ++i) {
		int pos = choices.indexOf (preselected[i]);
		if (pos >= 0) input->item (pos)->setSelected (true);
	}

	QSize base_hint = minimumSizeHint ();
	int other_height = base_hint.height () - input->minimumSizeHint ().height ();	// height of all other things besides the list widget
	int ideal_height = other_height + qMax (input->sizeHintForRow (0)*input->count (), 50);
	// KDialog appears to be smart enough to limit this to the available screen geometry
	setInitialSize (QSize (base_hint.width (), ideal_height));

	connect (input, SIGNAL (itemSelectionChanged()), this, SLOT (updateState()));
	updateState ();
}

RKSelectListDialog::~RKSelectListDialog () {
	RK_TRACE (DIALOGS);
}

void RKSelectListDialog::updateState () {
	RK_TRACE (DIALOGS);

	// TODO is there no QListWidget::hasSelection()?
	if (input->selectedItems ().isEmpty ()) enableButtonOk (false);
	else enableButtonOk (true);
}

//static
QStringList RKSelectListDialog::doSelect (QWidget *parent, const QString &caption, const QStringList& choices, const QStringList& preselected, bool multiple) {
	RK_TRACE (DIALOGS);

	RKSelectListDialog *dialog = new RKSelectListDialog (parent, caption, choices, preselected, multiple);
	int res = dialog->exec ();
	if (res != QDialog::Accepted) return QStringList ();

	QStringList list;
	QList<QListWidgetItem*> selected = dialog->input->selectedItems ();
	for (int i = 0; i < selected.length (); ++i) {
		list.append (selected[i]->text ());
	}

	return (list);
}

#include "rkselectlistdialog.moc"
