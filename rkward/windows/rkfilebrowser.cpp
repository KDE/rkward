/*
rkfilebrowser - This file is part of RKWard (https://rkward.kde.org). Created: Thu Apr 26 2007
SPDX-FileCopyrightText: 2007-2018 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkfilebrowser.h"

#include <kdiroperator.h>
#include <kurlcombobox.h>
#include <kurlcompletion.h>
#include <kcompletionbox.h>
#include <ktoolbar.h>
#include <kactioncollection.h>
#include <kconfiggroup.h>
#include <KSharedConfig>
#include <kfileitemactions.h>
#include <kfileitemlistproperties.h>
#include <KLocalizedString>
#include <kio/copyjob.h>
#include <kio_version.h>

#include <qdir.h>
#include <qlayout.h>
#include <QEvent>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QMenu>
#include <QInputDialog>

#include "rkworkplace.h"
#include "../rbackend/rkrinterface.h"
#include "../rkward.h"
#include "../misc/rkdummypart.h"

#include "../debug.h"

// static
RKFileBrowser *RKFileBrowser::main_browser = nullptr;

RKFileBrowser::RKFileBrowser (QWidget *parent, bool tool_window, const char *name) : RKMDIWindow (parent, FileBrowserWindow, tool_window, name) {
	RK_TRACE (APP);

	real_widget = nullptr;

	QVBoxLayout *layout = new QVBoxLayout (this);
	layout->setContentsMargins (0, 0, 0, 0);
	layout_widget = new QWidget (this);
	layout->addWidget (layout_widget);
	layout_widget->setFocusPolicy (Qt::StrongFocus);

	RKDummyPart *part = new RKDummyPart (this, layout_widget);
	setPart (part);
	initializeActivationSignals ();
}

RKFileBrowser::~RKFileBrowser () {
	RK_TRACE (APP);

	hide ();
}

void RKFileBrowser::showEvent (QShowEvent *e) {
	RK_TRACE (APP);

	if (!real_widget) {
		RK_DEBUG (APP, DL_INFO, "creating file browser");

		real_widget = new RKFileBrowserWidget (layout_widget);
		QVBoxLayout *l = new QVBoxLayout (layout_widget);
		l->setContentsMargins (0, 0, 0, 0);
		l->addWidget (real_widget);
		setFocusProxy (real_widget);
	}

	RKMDIWindow::showEvent (e);
	real_widget->syncToWD();
}

/////////////////// RKFileBrowserWidget ////////////////////

RKFileBrowserWidget::RKFileBrowserWidget (QWidget *parent) : QWidget (parent) {
	RK_TRACE (APP);

	QVBoxLayout *layout = new QVBoxLayout (this);

	KToolBar *toolbar = new KToolBar (this);
	toolbar->setIconSize (QSize (16, 16));
	toolbar->setToolButtonStyle (Qt::ToolButtonIconOnly);
	layout->addWidget (toolbar);

	urlbox = new KUrlComboBox (KUrlComboBox::Directories, true, this);
	KUrlCompletion* cmpl = new KUrlCompletion (KUrlCompletion::DirCompletion);
	urlbox->setCompletionObject (cmpl);
	urlbox->setAutoDeleteCompletionObject (true);
	urlbox->setSizePolicy (QSizePolicy (QSizePolicy::Expanding, QSizePolicy::Fixed));
	urlbox->completionBox (true)->installEventFilter (this);
	setFocusProxy (urlbox);
	layout->addWidget (urlbox);

	dir = new KDirOperator (QUrl (), this);
	dir->setPreviewWidget (nullptr);
	KConfigGroup config = KSharedConfig::openConfig ()->group ("file browser window");
	dir->readConfig (config);
	dir->setViewMode (KFile::Tree);
	connect (RKWardMainWindow::getMain (), &RKWardMainWindow::aboutToQuitRKWard, this, &RKFileBrowserWidget::saveConfig);
	layout->addWidget (dir);

	toolbar->addAction (dir->action (KDirOperator::Up));
	toolbar->addAction (dir->action (KDirOperator::Back));
	toolbar->addAction (dir->action (KDirOperator::Forward));
	toolbar->addAction (dir->action (KDirOperator::Home));

	QAction* action = new QAction (QIcon::fromTheme ("folder-sync"), i18n ("Working directory"), this);
	action->setToolTip (action->text ());
	connect(action, &QAction::triggered, this, [=] () { follow_working_directory = true; syncToWD(); });
	toolbar->addAction (action);
	toolbar->addSeparator ();
	toolbar->addAction (dir->action (KDirOperator::ShortView));
	toolbar->addAction (dir->action (KDirOperator::TreeView));
	toolbar->addAction (dir->action (KDirOperator::DetailedView));
//	toolbar->addAction (dir->action (KDirOperator::DetailedTreeView));	// should we have this as well? Trying to avoid crowding in the toolbar

	fi_actions = new KFileItemActions (this);
	rename_action = new QAction (i18n ("Rename"), this);  // Oh my, why isn't there a standard action for this?
	rename_action->setIcon (QIcon::fromTheme (QStringLiteral("edit-rename")));
	connect (rename_action, &QAction::triggered, this, &RKFileBrowserWidget::rename);
	connect (dir, &KDirOperator::contextMenuAboutToShow, this, &RKFileBrowserWidget::contextMenuHook);

	connect (dir, &KDirOperator::urlEntered, this, &RKFileBrowserWidget::urlChangedInView);
	connect (urlbox, static_cast<void (KUrlComboBox::*)(const QString&)>(&KUrlComboBox::returnPressed), this, &RKFileBrowserWidget::stringChangedInCombo);
	connect (urlbox, &KUrlComboBox::urlActivated, this, &RKFileBrowserWidget::urlChangedInCombo);

	connect (dir, &KDirOperator::fileSelected, this, &RKFileBrowserWidget::fileActivated);
	connect (RInterface::instance(), &RInterface::backendWorkdirChanged, this, &RKFileBrowserWidget::syncToWD);

	setURL (QUrl::fromLocalFile (QDir::currentPath ()));
}

RKFileBrowserWidget::~RKFileBrowserWidget () {
	RK_TRACE (APP);
}

void RKFileBrowserWidget::syncToWD () {
	RK_TRACE (APP);

	if (!follow_working_directory) return;
	if (isVisible()) setURL (QUrl::fromLocalFile (QDir::currentPath ()));
}

void RKFileBrowserWidget::rename () {
	RK_TRACE (APP);

	QString name = QInputDialog::getText (this, i18n ("Rename..."), i18n ("New name for '%1':", context_menu_url.fileName ()), QLineEdit::Normal, context_menu_url.fileName ());
	if (name.isEmpty ()) return;

	QUrl dest_url = context_menu_url;
	dest_url.setPath (context_menu_url.adjusted (QUrl::RemoveFilename).path () + '/' + name);
	KIO::moveAs (context_menu_url, dest_url);
}

void RKFileBrowserWidget::contextMenuHook(const KFileItem& item, QMenu* menu) {
	RK_TRACE (APP);

	QList<KFileItem> dummy;
	dummy.append (item);
	fi_actions->setItemListProperties (KFileItemListProperties (dummy));
	context_menu_url = item.url ();

	// some versions of KDE appear to re-use the actions, others don't, and yet other are just plain broken (see this thread: https://mail.kde.org/pipermail/rkward-devel/2011-March/002770.html)
	// Therefore, we remove all actions, explicitly, each time the menu is shown, then add them again.
	QList<QAction*> menu_actions = menu->actions ();
	QAction *first_sep = nullptr;
	for (QAction* act : std::as_const(menu_actions)) {
		if (added_service_actions.contains (act)) menu->removeAction (act);
		if (!first_sep && act->isSeparator ()) first_sep = act;
	}
	added_service_actions.clear ();
	menu_actions = menu->actions ();

	menu->insertAction (first_sep, rename_action);
	fi_actions->insertOpenWithActionsTo(nullptr, menu, QStringList());
	fi_actions->addActionsTo(menu);

	const QList<QAction*> menu_actions_after = menu->actions ();
	for (QAction* act : menu_actions_after) if (!menu_actions.contains (act)) added_service_actions.append (act);
}

// does not work in d-tor. Apparently it's too late, then
void RKFileBrowserWidget::saveConfig () {
	RK_TRACE (APP);

	KConfigGroup config = KSharedConfig::openConfig ()->group ("file browser window");
	dir->writeConfig (config);
}

void RKFileBrowserWidget::setURL (const QUrl &url) {
	RK_TRACE (APP);

	urlbox->setUrl (url);
	dir->setUrl (url, true);
	follow_working_directory = (url.path() == QDir::currentPath ());
}

void RKFileBrowserWidget::urlChangedInView (const QUrl &url) {
	RK_TRACE (APP);

	urlbox->setUrl (url);
	follow_working_directory = (url.path() == QDir::currentPath ());
}

void RKFileBrowserWidget::stringChangedInCombo (const QString &url) {
	RK_TRACE (APP);

	dir->setUrl (QUrl::fromUserInput (url, QDir::currentPath (), QUrl::AssumeLocalFile), true);
}

void RKFileBrowserWidget::urlChangedInCombo (const QUrl &url) {
	RK_TRACE (APP);

	dir->setUrl (url, true);
}

bool RKFileBrowserWidget::eventFilter (QObject* o, QEvent* e) {
	// don't trace here

	if (o == urlbox->completionBox () && e->type () == QEvent::Resize) {
		RK_TRACE (APP);
		// this hack (originally from a KDE 3 version of kate allows the completion popup to span beyond the border of the filebrowser widget itself

		KCompletionBox* box = urlbox->completionBox ();
		RK_ASSERT (box);

		int add = box->verticalScrollBar ()->isVisible () ? box->verticalScrollBar ()->width () : 0;
		box->setMinimumWidth (qMin (topLevelWidget ()->width (), box->sizeHintForColumn (0) + add));

		return false;
	}

	return (QWidget::eventFilter (o, e));
}

void RKFileBrowserWidget::fileActivated (const KFileItem& item) {
	RK_TRACE (APP);

	RKWorkplace::mainWorkplace ()->openAnyUrl (item.url ());
}

