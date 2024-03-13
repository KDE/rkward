/*
rkxmlguipreviewarea - This file is part of RKWard (https://rkward.kde.org). Created: Wed Feb 03 2016
SPDX-FileCopyrightText: 2016-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkxmlguipreviewarea.h"

#include <QMenu>
#include <QToolButton>
#include <QEvent>
#include <QMenuBar>
#include <QWidgetAction>
#include <QLabel>
#include <QVBoxLayout>
#include <QDomElement>

#include <KXMLGUIFactory>
#include <KXMLGUIBuilder>
#include <KToolBar>
#include <KLocalizedString>

#include "../windows/rkmdiwindow.h"
#include "../windows/rkworkplace.h"
#include "../rbackend/rcommand.h"
#include "../rbackend/rkrinterface.h"
#include "rkstandardicons.h"

#include "../debug.h"

class RKXMLGUIPreviewBuilder : public KXMLGUIBuilder {
public:
	RKXMLGUIPreviewBuilder(QWidget* parent, QMenuBar* menubar) : KXMLGUIBuilder(parent), menubar(menubar) {};
	QStringList containerTags() const override {
		QStringList ret;
		ret << QStringLiteral("menubar") << QStringLiteral("menu");
		return ret;
	}
	QWidget* createContainer(QWidget *parent, int index, const QDomElement &element, QAction *&containerAction) override {
		QString tagname = element.tagName().toLower();
		if (tagname == QStringLiteral("menubar")) {
			return menubar;
		}
		// only menus will actually be built, as we effectively disabled toolbars via containerTags(), above
		return KXMLGUIBuilder::createContainer(parent, index, element, containerAction);
	}
	void removeContainer(QWidget *container, QWidget *parent, QDomElement &element, QAction *containerAction) override {
		if (container == menubar) {
			menubar->clear();
			return; // do not delete it
		}
		KXMLGUIBuilder::removeContainer(container, parent, element, containerAction);
	}
private:
	QMenuBar *menubar;
};

RKXMLGUIPreviewArea::RKXMLGUIPreviewArea (const QString &label, QWidget* parent) : QWidget (parent) {
	RK_TRACE (PLUGIN);

	current = nullptr;
	menubar = nullptr;
	builder = nullptr;
	factory = nullptr;

	QVBoxLayout *vl = new QVBoxLayout(this);
	vl->setContentsMargins(0, 0, 0, 0);
	QFrame *line = new QFrame();
	line->setFrameShape(QFrame::HLine);
	vl->addWidget(line);
	QHBoxLayout *hl = new QHBoxLayout();
	vl->addLayout(hl);
	lab = new QLabel(label);
	QFont fnt(lab->font());
	fnt.setBold(true);
	lab->setFont(fnt);
	lab->setAlignment(Qt::AlignCenter);
	QToolButton *tb = new QToolButton();
	tb->setAutoRaise(true);
	tb->setIcon(RKStandardIcons::getIcon(RKStandardIcons::ActionDelete));
	connect(tb, &QAbstractButton::clicked, this, [this]() { hide(); Q_EMIT previewClosed(this); });

	QToolButton *menu_button = new QToolButton(this);
	menu_button->setPopupMode(QToolButton::InstantPopup);
	menu_button->setIcon(RKStandardIcons::getIcon(RKStandardIcons::ActionShowMenu));
	menu_button->setMenu(menu = new QMenu(this));
	connect(menu, &QMenu::aboutToShow, this, &RKXMLGUIPreviewArea::prepareMenu);

	hl->addWidget(menu_button);
	hl->addStretch();
	hl->addWidget(lab);
	hl->addWidget(tb);
	hl->addStretch();
	internal_layout = new QVBoxLayout(this);
	vl->addLayout(internal_layout);

	menubar = new QMenuBar(nullptr); // it is important that the menubar never is a child of the main window, not even indirectly! https://bugs.kde.org/show_bug.cgi?id=416911
	builder = new RKXMLGUIPreviewBuilder(this, menubar);
	factory = new KXMLGUIFactory(builder, this);
}

RKXMLGUIPreviewArea::~RKXMLGUIPreviewArea () {
	RK_TRACE (PLUGIN);

	if (current && factory) {
		factory->removeClient(current);
		current->setFactory (0);
	}
	delete menubar;
	delete builder;
	delete factory;
}

void RKXMLGUIPreviewArea::setLabel (const QString& label) {
	RK_TRACE(PLUGIN);
	lab->setText(label);
}

QString RKXMLGUIPreviewArea::label() const {
	RK_TRACE(PLUGIN);
	return lab->text();
}

void RKXMLGUIPreviewArea::setWindow(RKMDIWindow* window) {
	RK_TRACE(PLUGIN);

	if (current) {
		factory->removeClient(current);  // _always_ remove before adding, or the previous child will be leaked in the factory
	}
	window->setWindowStyleHint("preview");
	current = window->getPart();
	internal_layout->addWidget(window);
	factory->addClient(current);
	QList<QAction*> acts = actions();
	for (int i = 0; i < acts.size (); ++i) acts[i]->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	RKWorkplace::mainWorkplace()->setWindowNotManaged(window);
}

void RKXMLGUIPreviewArea::prepareMenu () {
	RK_TRACE (PLUGIN);

	// flatten menu, and try to purge irrelevant actions
	menu->clear ();
	QList<QAction*> entries = menubar->actions ();
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
	id = "0x" + QString::number((quint64) (quintptr) this, 16);
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
		if (!warnings.isEmpty ()) warnings = QString("<b>%1</b>\n<pre>%2</pre>").arg(i18n("Warnings or Errors:"), warnings.toHtmlEscaped());
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
	RInterface::whenAllFinished(this, [this]() { setStatusMessage(shortStatusLabel()); });

	RInterface::issueCommand (command);
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

	Q_EMIT statusChanged();
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
