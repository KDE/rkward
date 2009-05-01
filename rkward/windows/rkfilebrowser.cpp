/***************************************************************************
                          rkfilebrowser  -  description
                             -------------------
    begin                : Thu Apr 26 2007
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

#include "rkfilebrowser.h"

#include <kdiroperator.h>
#include <kurlcombobox.h>
#include <kurlcompletion.h>
#include <ktoolbar.h>
#include <krun.h>

#include <qdir.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qlistbox.h>

#include "rkworkplace.h"
#include "../rkward.h"
#include "../misc/rkdummypart.h"

#include "../debug.h"

// static
RKFileBrowser *RKFileBrowser::main_browser = 0;

RKFileBrowser::RKFileBrowser (QWidget *parent, bool tool_window, const char *name) : RKMDIWindow (parent, FileBrowserWindow, tool_window, name) {
	RK_TRACE (APP);

	real_widget = 0;

	QVBoxLayout *layout = new QVBoxLayout (this);
	layout_widget = new QVBox (this);
	layout->addWidget (layout_widget);
	layout_widget->setFocusPolicy (QWidget::StrongFocus);

	RKDummyPart *part = new RKDummyPart (this, layout_widget);
	setPart (part);
	initializeActivationSignals ();
}

RKFileBrowser::~RKFileBrowser () {
	RK_TRACE (APP);

	hide ();
}

void RKFileBrowser::show () {
	RK_TRACE (APP);

	if (!real_widget) {
		RK_DO (qDebug ("creating file browser"), APP, DL_INFO);

		real_widget = new RKFileBrowserWidget (layout_widget);
		layout_widget->setFocusProxy (real_widget);
	}

	RKMDIWindow::show ();
}

void RKFileBrowser::currentWDChanged () {
	RK_TRACE (APP);
}



RKFileBrowserWidget::RKFileBrowserWidget (QWidget *parent) : QVBox (parent) {
	RK_TRACE (APP);

	KToolBar *toolbar = new KToolBar (this);
	toolbar->setIconSize (16);

	urlbox = new KURLComboBox (KURLComboBox::Directories, true, this);
	KURLCompletion* cmpl = new KURLCompletion (KURLCompletion::DirCompletion);
	urlbox->setCompletionObject (cmpl);
	urlbox->setAutoDeleteCompletionObject (true);
	urlbox->setSizePolicy (QSizePolicy (QSizePolicy::Expanding, QSizePolicy::Fixed));
	urlbox->listBox ()->installEventFilter (this);

// Workaround: KDirOperator sneaks its actions into its toplevel QWidget. Among those "trash", bound to the Del key, and
// that can lead to ugly results (see http://sourceforge.net/tracker/?func=detail&aid=2033636&group_id=50231&atid=459007).
// For good measure we deactivate all shortcuts.
	dir = new KDirOperator (KURL (), this);
	for (int i = 0; i < dir->actionCollection ()->count (); ++i) {
		dir->actionCollection ()->action (i)->setShortcut (KShortcut::null ());
	}
	dir->setView(KFile::Simple);
	dir->setPreviewWidget (0);

	dir->actionCollection ()->action ("up")->plug (toolbar);
	dir->actionCollection ()->action ("back")->plug (toolbar);
	dir->actionCollection ()->action ("forward")->plug (toolbar);
	dir->actionCollection ()->action ("home")->plug (toolbar);
	dir->actionCollection ()->action ("short view")->plug (toolbar);
	dir->actionCollection ()->action ("detailed view")->plug (toolbar);

	connect (dir, SIGNAL (urlEntered (const KURL &)), this, SLOT (urlChangedInView (const KURL &)));
	connect (urlbox, SIGNAL (returnPressed (const QString &)), this, SLOT (urlChangedInCombo (const QString &)));
	connect (urlbox, SIGNAL (urlActivated (const KURL&)), this, SLOT (urlChangedInCombo (const KURL&)));

	connect (dir, SIGNAL (fileSelected (const KFileItem*)), this, SLOT (fileActivated (const KFileItem*)));

	setURL (QDir::currentDirPath ());
}

RKFileBrowserWidget::~RKFileBrowserWidget () {
	RK_TRACE (APP);
}

void RKFileBrowserWidget::setURL (const QString &url) {
	RK_TRACE (APP);

	urlbox->setURL (url);
	dir->setURL (url, true);
}

void RKFileBrowserWidget::urlChangedInView (const KURL &url) {
	RK_TRACE (APP);

	urlbox->setURL (url);
}

void RKFileBrowserWidget::urlChangedInCombo (const QString &url) {
	RK_TRACE (APP);

	dir->setURL (url, true);
}

void RKFileBrowserWidget::urlChangedInCombo (const KURL &url) {
	RK_TRACE (APP);

	dir->setURL (url, true);
}

bool RKFileBrowserWidget::eventFilter (QObject *watched, QEvent *e) {
	// don't trace

	// fix size of popup (copied from katefileselector.cpp)
	QListBox *lb = urlbox->listBox ();
	if (watched == lb && e->type() == QEvent::Show) {
		int add = lb->height() < lb->contentsHeight() ? lb->verticalScrollBar()->width() : 0;
		int w = QMIN (topLevelWidget ()->width(), lb->contentsWidth() + add);
		lb->resize (w, lb->height());
	}
	return QWidget::eventFilter (watched, e);
}

void RKFileBrowserWidget::fileActivated (const KFileItem *item) {
	RK_TRACE (APP);

	RK_ASSERT (item);

	QString mimetype = item->mimetype ();
	if (mimetype.startsWith ("text/")) {
		if (mimetype == "text/html") {
			RKWorkplace::mainWorkplace ()->openHelpWindow (item->url (), true);
		} else {
			RKWorkplace::mainWorkplace ()->openScriptEditor (item->url ());
		}
	} else {
		if (item->name (true).endsWith (".rdata")) {
			RKWardMainWindow::getMain ()->fileOpenAskSave (item->url ());
		} else {
			new KRun (item->url (), item->mode (), item->isLocalFile ());
		}
	}
}

#include "rkfilebrowser.moc"
