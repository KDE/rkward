/***************************************************************************
                          rksaveobjectchooser  -  description
                             -------------------
    begin                : Mon Nov 27 2006
    copyright            : (C) 2006, 2007, 2009, 2010 by Thomas Friedrichsmeier
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

#include "rksaveobjectchooser.h"

#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTimer>

#include <klocale.h>
#include <kdialog.h>
#include <kvbox.h>

#include "../core/robjectlist.h"
#include "../core/renvironmentobject.h"
#include "../misc/rkobjectlistview.h"

#include "../debug.h"

RKSaveObjectChooser::RKSaveObjectChooser (QWidget *parent, const QString &initial) : QWidget (parent), RObjectListener (RObjectListener::Other) {
	RK_TRACE (MISC);

	object_exists = false;
	addNotificationType (RObjectListener::ObjectRemoved);
	addNotificationType (RObjectListener::ChildAdded);
	root_object = 0;
	current_object = 0;

	QVBoxLayout *layout = new QVBoxLayout (this);
	layout->setContentsMargins (0, 0, 0, 0);

	QHBoxLayout *hlayout = new QHBoxLayout ();
	root_label = new QLabel (this);
	hlayout->addWidget (root_label);
	hlayout->addStretch ();
	root_button = new QPushButton (i18n ("Change"), this);
	connect (root_button, SIGNAL (clicked()), this, SLOT (selectRootObject()));
	hlayout->addWidget (root_button);
	layout->addLayout (hlayout);

	name_edit = new QLineEdit (this);
	name_edit->setText (initial);
	connect (name_edit, SIGNAL (textChanged (const QString &)), this, SLOT (updateState()));
	layout->addWidget (name_edit);

	overwrite_confirm = new QCheckBox (this);
	connect (overwrite_confirm, SIGNAL (stateChanged (int)), this, SLOT (updateState()));
	layout->addWidget (overwrite_confirm);

	// initialize
	setRootObject (0);
}

RKSaveObjectChooser::~RKSaveObjectChooser () {
	RK_TRACE (MISC);

	if (current_object) stopListenForObject (current_object);
	stopListenForObject (root_object);
}

void RKSaveObjectChooser::setRootObject (RObject* new_root) {
	RK_TRACE (MISC);

	if (root_object && (new_root == root_object)) {
		return;
	}

	if (root_object) stopListenForObject (root_object);
	if (!new_root) new_root = RObjectList::getGlobalEnv ();
	else if (!new_root->isInGlobalEnv ()) new_root = RObjectList::getGlobalEnv ();
	else if (!new_root->isContainer ()) new_root = RObjectList::getGlobalEnv ();
	root_object = new_root;
	listenForObject (root_object);

	root_label->setText (i18n ("Parent object: %1", root_object->getShortName ()));

	updateState ();
}

void RKSaveObjectChooser::selectRootObject () {
	RK_TRACE (MISC);

	// TODO: not very pretty, yet
	KDialog *dialog = new KDialog (this);
	dialog->setButtons (KDialog::Ok|KDialog::Cancel);
	dialog->setCaption (i18n ("Select parent object"));
	dialog->setModal (true);
	KVBox *page = new KVBox (dialog);
	dialog->setMainWidget (page);

	RKObjectListView* list_view = new RKObjectListView (page);
	list_view->setSelectionMode (QAbstractItemView::SingleSelection);
	list_view->getSettings ()->setSetting (RKObjectListViewSettings::ShowObjectsAllEnvironments, false);
	list_view->initialize ();
	list_view->setObjectCurrent (root_object);
	connect (list_view, SIGNAL (doubleClicked (const QModelIndex&)), dialog, SLOT (accept()));

	dialog->exec ();

	if (dialog->result () == QDialog::Accepted) {
		RObject::ObjectList sel = list_view->selectedObjects ();
		if (sel.isEmpty ()) setRootObject (0);
		else {
			RK_ASSERT (sel.size () == 1);
			setRootObject (sel[0]);
		}
	}

	delete dialog;
}

void RKSaveObjectChooser::objectRemoved (RObject* removed) {
	RK_TRACE (MISC);

	if (removed == root_object) {
		setRootObject (0);
	} else if (removed == current_object) {
		stopListenForObject (removed);
		current_full_name.clear ();	// hack to achieve proper emittance of change signal
		QTimer::singleShot (0, this, SLOT (updateState()));
	} else {
		RK_ASSERT (false);
	}
}

void RKSaveObjectChooser::childAdded (int, RObject* parent) {
	RK_TRACE (MISC);

	if (parent == root_object) {
		updateState ();
	} else {
		// unusual, but ok
		RK_ASSERT (parent == current_object);
	}
}

void RKSaveObjectChooser::setBaseName (const QString &name) {
	RK_TRACE (MISC);

	name_edit->setText (name);
	updateState ();
}

bool RKSaveObjectChooser::isOk () const {
	RK_TRACE (MISC);

	if (name_edit->text ().isEmpty ()) return false;
	return ((!object_exists) || overwrite_confirm->isChecked ());
}

void RKSaveObjectChooser::updateState () {
	RK_TRACE (MISC);
	RK_ASSERT (root_object && root_object->isContainer ());

	QString new_name = static_cast<RContainerObject*> (root_object)->validizeName (name_edit->text (), false);
	if (current_object) stopListenForObject (current_object);
	current_object = root_object->findObject (new_name);
	new_name = root_object->makeChildName (new_name);	// make it the full name
	if (current_object) {
		object_exists = true;
		overwrite_confirm->setText (i18n ("Overwrite? (The given object name already exists)"));
		overwrite_confirm->setEnabled (true);
		listenForObject (current_object);
	} else {
		object_exists = false;
		overwrite_confirm->setText (i18n ("Overwrite?"));
		overwrite_confirm->setEnabled (false);
		overwrite_confirm->setChecked (false);
	}

	if ((new_name != current_full_name) || (sender () == overwrite_confirm)) {
		current_full_name = new_name;
		emit (changed (isOk ()));
	}
}

void RKSaveObjectChooser::setBackgroundColor (const QColor &color) {
	RK_TRACE (MISC);

	QPalette palette = name_edit->palette ();
	palette.setColor (name_edit->backgroundRole (), color);
	name_edit->setPalette (palette);
}

QString RKSaveObjectChooser::currentBaseName () const {
	return name_edit->text ();
}

#include "rksaveobjectchooser.moc"
