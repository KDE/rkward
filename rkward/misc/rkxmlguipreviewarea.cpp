/***************************************************************************
                          rkxmlguipreviewarea  -  description
                             -------------------
    begin                : Wed Feb 03 2016
    copyright            : (C) 2016 by Thomas Friedrichsmeier
    email                : thomas.friedrichsmeier@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "rkxmlguipreviewarea.h"

#include <QMenu>
#include <QToolButton>
#include <QEvent>
#include <QMenuBar>
#include <QWidgetAction>
#include <QLabel>
#include <QVBoxLayout>

#include <kxmlguifactory.h>
#include <ktoolbar.h>
#include <KLocalizedString>

#include "../windows/rkmdiwindow.h"
#include "../rbackend/rcommand.h"
#include "../rbackend/rkrinterface.h"
#include "../rkglobals.h"
#include "rkstandardicons.h"

#include "../debug.h"

RKXMLGUIPreviewArea::RKXMLGUIPreviewArea (const QString &label, QWidget* parent) : KXmlGuiWindow (parent) {
	RK_TRACE (PLUGIN);

	_label = label;
	wrapper_widget = 0;
	current = 0;
	setWindowFlags (Qt::Widget);
	setMenuBar (new QMenuBar (this));
	setHelpMenuEnabled (false);
}

RKXMLGUIPreviewArea::~RKXMLGUIPreviewArea () {
	RK_TRACE (PLUGIN);

	if (current) {
		removeChildClient (current);
		current->setFactory (0);
	}
}

void RKXMLGUIPreviewArea::setLabel (const QString& label) {
	RK_TRACE (PLUGIN);

	if (label == _label) return;
	_label = label;
	if (wrapper_widget) {
		lab->setText (label);
	}
}

QWidget* RKXMLGUIPreviewArea::wrapperWidget () {
	if (wrapper_widget) return wrapper_widget;

	wrapper_widget = new QWidget ();

	QVBoxLayout *vl = new QVBoxLayout (wrapper_widget);
	vl->setContentsMargins (0, 0, 0, 0);
	QFrame *line = new QFrame (wrapper_widget);
	line->setFrameShape (QFrame::HLine);
	vl->addWidget (line);
	QHBoxLayout *hl = new QHBoxLayout ();
	vl->addLayout (hl);
	lab = new QLabel (_label, wrapper_widget);
	QFont fnt (lab->font ());
	fnt.setBold (true);
	lab->setFont (fnt);
	lab->setAlignment (Qt::AlignCenter);
	QToolButton *tb = new QToolButton (wrapper_widget);
	tb->setAutoRaise (true);
	tb->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionDelete));
	connect (tb, &QAbstractButton::clicked, [this]() { wrapper_widget->hide (); emit (previewClosed(this)); });

	QToolButton *menu_button = new QToolButton (this);
	menu_button->setPopupMode (QToolButton::InstantPopup);
	menu_button->setIcon (RKStandardIcons::getIcon (RKStandardIcons::ActionShowMenu));
	menu_button->setMenu (menu = new QMenu ());
	connect (menu, &QMenu::aboutToShow, this, &RKXMLGUIPreviewArea::prepareMenu);

	hl->addWidget (menu_button);
	hl->addStretch ();
	hl->addWidget (lab);
	hl->addWidget (tb);
	hl->addStretch ();

	vl->addWidget (this);
	show ();

	return wrapper_widget;
}

void RKXMLGUIPreviewArea::childEvent (QChildEvent *event) {
	RK_TRACE (PLUGIN);

	if (event->type () == QEvent::ChildAdded) {
		RKMDIWindow *child = qobject_cast<RKMDIWindow*> (event->child ());
		if (child) {
			if (current) {
				removeChildClient (current);
				factory ()->removeClient (current);  // _always_ remove before adding, or the previous child will be leaked in the factory
			}
			child->setWindowStyleHint ("preview");
			current = child->getPart ();
			insertChildClient (current);
			setCentralWidget (child);
			createGUI ("rkwrapper_widgetpart.rc");
			menuBar ()->hide ();
			QList<KToolBar*> tbars = toolBars ();
			for (int i = 0; i < tbars.size (); ++i) tbars[i]->hide ();
			// avoid shortcut conflicts
			QList<QAction*> acts = actions ();
			for (int i = 0; i < acts.size (); ++i) acts[i]->setShortcutContext(Qt::WidgetWithChildrenShortcut);
		}
	}
	QObject::childEvent (event);
}

void RKXMLGUIPreviewArea::prepareMenu () {
	RK_TRACE (PLUGIN);

	// flatten menu, and try to purge irrelevant actions
	menu->clear ();
	QList<QAction*> entries = menuBar ()->actions ();
	for (int i = 0; i < entries.size (); ++i) {
		QMenu *smenu = entries[i]->menu ();
		if (!smenu) continue;    // Don't think it can happen...
		if (entries[i]->objectName () == "settings") continue;  // Skip settings menu, entirely

		QList<QAction*> subentries = smenu->actions ();
		QList<QAction*> entries_to_add;
		bool menu_empty = true;
		for (int j = 0; j < subentries.size (); ++j) {
			QAction *act = subentries[j];
			if (act->isVisible () && act->isEnabled () && act) {
				entries_to_add.append (act);
				if (!act->isSeparator ()) menu_empty = false;  // Copy separators, but purge menus with only separators in them.
			}
		}
		if (menu_empty) continue;

		QWidgetAction *act = new QWidgetAction (this);
		QLabel *lab = new QLabel ("<b>" + entries[i]->text ().remove ('&') + "</b>");
		lab->setAlignment (Qt::AlignCenter);
		act->setDefaultWidget (lab);
		menu->addAction (act);

		QMenu *where_to_add = menu;
		if (entries_to_add.size () >= 8) {                     // if there are really many entries in the menu don't flatten it, keep it as a (shortened) submenu
			where_to_add = menu->addMenu (entries[i]->text ());
		}
		for (int j = 0; j < entries_to_add.size (); ++j) {
			where_to_add->addAction (entries_to_add[j]);
		}
	}
}


#include "../windows/rkworkplace.h"

RKPreviewManager::RKPreviewManager(QObject* parent) : QObject (parent) {
	RK_TRACE (PLUGIN);

	update_pending = NoUpdatePending;
	updating = false;
	id = QString ().sprintf ("%p", this).remove ('%');
}

RKPreviewManager::~RKPreviewManager () {
	RK_TRACE (PLUGIN);
}

void RKPreviewManager::previewCommandDone (RCommand* command) {
	RK_TRACE (PLUGIN);

	updating = false;
	if (update_pending == NoUpdatePossible) {
		setNoPreviewAvailable ();
	} else {
		QString warnings = command->warnings () + command->error ();
		if (!warnings.isEmpty ()) warnings = QString ("<b>%1</b>\n<pre>%2</pre>").arg (i18n ("Warnings or Errors:")).arg (warnings.toHtmlEscaped ());
		setStatusMessage (warnings);
	}
}

void RKPreviewManager::setCommand (RCommand* command) {
	RK_TRACE (PLUGIN);

	RK_ASSERT (!updating);
	updating = true;
	update_pending = NoUpdatePending;
	connect (command->notifier(), &RCommandNotifier::commandFinished, this, &RKPreviewManager::previewCommandDone);

	// Send an empty dummy command first. This is to sync up with any commands that should have been run _before_ the preview (e.g. to set up the preview area, so that status labels can be shown)
	RCommand *dummy = new RCommand (QString (), RCommand::App | RCommand::Sync | RCommand::EmptyCommand);
	connect (dummy->notifier(), &RCommandNotifier::commandFinished, [this]() { setStatusMessage (shortStatusLabel ()); });
	RKGlobals::rInterface ()->issueCommand (dummy);

	RKGlobals::rInterface ()->issueCommand (command);
	setStatusMessage (shortStatusLabel ());
}

void RKPreviewManager::setUpdatePending () {
	if (update_pending == UpdatePending) return;
	RK_TRACE (PLUGIN);

	update_pending = UpdatePending;
	setStatusMessage (shortStatusLabel ());
}

void RKPreviewManager::setNoPreviewAvailable () {
	if (update_pending == NoUpdatePossible) return;
	RK_TRACE (PLUGIN);

	update_pending = NoUpdatePossible;
	setStatusMessage (shortStatusLabel ());
}

void RKPreviewManager::setPreviewDisabled () {
	if (update_pending == PreviewDisabled) return;
	RK_TRACE (PLUGIN);

	update_pending = PreviewDisabled;
	setStatusMessage (shortStatusLabel ());
}

void RKPreviewManager::setStatusMessage (const QString& message) {
	RK_TRACE (PLUGIN);

	RKMDIWindow *window = RKWorkplace::mainWorkplace ()->getNamedWindow (id);
	if (window) window->setStatusMessage (message);

	emit (statusChanged());
}

QString RKPreviewManager::shortStatusLabel() const {
//	RK_TRACE (PLUGIN);

	if (update_pending == NoUpdatePossible) {
		return (i18n ("Preview not (yet) possible"));
	} else if (update_pending == PreviewDisabled) {
		return (i18n ("Preview disabled"));
	} else if (updating || (update_pending == UpdatePending)) {
		return (i18n ("Preview updating"));
	} else {
		return (i18n ("Preview up to date"));
	}
}

#include "rkxmlguipreviewarea.moc"
