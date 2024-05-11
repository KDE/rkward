/*
rksaveobjectchooser - This file is part of RKWard (https://rkward.kde.org). Created: Mon Nov 27 2006
SPDX-FileCopyrightText: 2006-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rksaveobjectchooser.h"

#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTimer>
#include <QDialog>
#include <QDialogButtonBox>

#include <KLocalizedString>
#include <KMessageWidget>

#include "../core/robjectlist.h"
#include "../core/renvironmentobject.h"
#include "../misc/rkobjectlistview.h"

#include "../debug.h"

RKSaveObjectChooser::RKSaveObjectChooser (QWidget *parent, const QString &initial) : QWidget (parent), RObjectListener (RObjectListener::Other) {
	RK_TRACE (MISC);

	object_exists = false;
	addNotificationType (RObjectListener::ObjectRemoved);
	addNotificationType (RObjectListener::ChildAdded);
	root_object = nullptr;
	current_object = nullptr;

	QVBoxLayout *layout = new QVBoxLayout (this);
	layout->setContentsMargins (0, 0, 0, 0);

	QHBoxLayout *hlayout = new QHBoxLayout ();
	root_label = new QLabel (this);
	hlayout->addWidget (root_label);
	hlayout->addStretch ();
	root_button = new QPushButton (i18n ("Change"), this);
	connect (root_button, &QPushButton::clicked, this, &RKSaveObjectChooser::selectRootObject);
	hlayout->addWidget (root_button);
	layout->addLayout (hlayout);

	name_edit = new QLineEdit (this);
	name_edit->setText (initial);
	connect (name_edit, &QLineEdit::textChanged, this, &RKSaveObjectChooser::updateState);
	layout->addWidget (name_edit);

	overwrite_confirm = new QCheckBox(i18n("Overwrite"), this);
	connect(overwrite_confirm, &QCheckBox::stateChanged, this, &RKSaveObjectChooser::updateState);
	overwrite_warn = new KMessageWidget(i18n("The given object name already exists"));
	overwrite_warn->setCloseButtonVisible(false);
	hlayout = new QHBoxLayout();
	hlayout->addWidget(overwrite_confirm);
	hlayout->addWidget(overwrite_warn);
	layout->addLayout(hlayout);

	// initialize
	setRootObject(nullptr);
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
	QDialog *dialog = new QDialog (this);
	dialog->setWindowTitle (i18n ("Select parent object"));
	dialog->setModal (true);
	QVBoxLayout *layout = new QVBoxLayout (dialog);

	RKObjectListView* list_view = new RKObjectListView (false, dialog);
	list_view->setSelectionMode (QAbstractItemView::SingleSelection);
	list_view->initialize ();
	list_view->setObjectCurrent (root_object);
	connect (list_view, &QAbstractItemView::doubleClicked, dialog, &QDialog::accept);
	layout->addWidget (list_view);

	QDialogButtonBox *buttons = new QDialogButtonBox (QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dialog);
	connect (buttons->button (QDialogButtonBox::Ok), &QPushButton::clicked, dialog, &QDialog::accept);
	connect (buttons->button (QDialogButtonBox::Cancel), &QPushButton::clicked, dialog, &QDialog::reject);
	layout->addWidget (buttons);

	dialog->exec ();

	if (dialog->result () == QDialog::Accepted) {
		RObject::ObjectList sel = list_view->selectedObjects ();
		if (sel.isEmpty()) setRootObject(nullptr);
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
		setRootObject(nullptr);
	} else if (removed == current_object) {
		stopListenForObject (removed);
		current_full_name.clear ();	// hack to achieve proper Q_EMIT of change signal
		QTimer::singleShot(0, this, &RKSaveObjectChooser::updateState);
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
		overwrite_confirm->setEnabled(true);
		overwrite_warn->setMessageType(overwrite_confirm->isChecked() ? KMessageWidget::Information : KMessageWidget::Error);
		overwrite_warn->animatedShow();
		listenForObject (current_object);
	} else {
		object_exists = false;
		overwrite_confirm->setEnabled(false);
		overwrite_confirm->setChecked(false);
		overwrite_warn->hide();
	}

	if ((new_name != current_full_name) || (sender () == overwrite_confirm)) {
		current_full_name = new_name;
		Q_EMIT changed(isOk());
	}
}

void RKSaveObjectChooser::setStyleSheet (const QString &style) {
	RK_TRACE (MISC);

	name_edit->setStyleSheet(style);
}

QString RKSaveObjectChooser::currentBaseName () const {
	return name_edit->text ();
}

