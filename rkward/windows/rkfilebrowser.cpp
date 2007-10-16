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
#include <kactioncollection.h>

#include <qdir.h>
#include <qlayout.h>
#include <q3vbox.h>
//Added by qt3to4:
#include <QEvent>
#include <QVBoxLayout>

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
	layout->setContentsMargins (0, 0, 0, 0);
	layout_widget = new KVBox (this);
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
		RK_DO (qDebug ("creating file browser"), APP, DL_INFO);

		real_widget = new RKFileBrowserWidget (layout_widget);
		layout_widget->setFocusProxy (real_widget);
	}

	RKMDIWindow::showEvent (e);
}

void RKFileBrowser::currentWDChanged () {
	RK_TRACE (APP);
}



RKFileBrowserWidget::RKFileBrowserWidget (QWidget *parent) : KVBox (parent) {
	RK_TRACE (APP);

	KToolBar *toolbar = new KToolBar (this);
	toolbar->setIconSize (QSize (16, 16));
	toolbar->setToolButtonStyle (Qt::ToolButtonIconOnly);

	urlbox = new KUrlComboBox (KUrlComboBox::Directories, true, this);
	KUrlCompletion* cmpl = new KUrlCompletion (KUrlCompletion::DirCompletion);
	urlbox->setCompletionObject (cmpl);
	urlbox->setAutoDeleteCompletionObject (true);
	urlbox->setSizePolicy (QSizePolicy (QSizePolicy::Expanding, QSizePolicy::Fixed));
// KDE4: do we need this (see eventFilter(), below)	urlbox->listBox ()->installEventFilter (this);

	dir = new KDirOperator (KUrl (), this);
	dir->setView(KFile::Simple);
	dir->setPreviewWidget (0);

	toolbar->addAction (dir->actionCollection ()->action ("up"));
	toolbar->addAction (dir->actionCollection ()->action ("back"));
	toolbar->addAction (dir->actionCollection ()->action ("forward"));
	toolbar->addAction (dir->actionCollection ()->action ("home"));
	toolbar->addAction (dir->actionCollection ()->action ("short view"));
	toolbar->addAction (dir->actionCollection ()->action ("detailed view"));

	connect (dir, SIGNAL (urlEntered (const KUrl &)), this, SLOT (urlChangedInView (const KUrl &)));
	connect (urlbox, SIGNAL (returnPressed (const QString &)), this, SLOT (urlChangedInCombo (const QString &)));
	connect (urlbox, SIGNAL (urlActivated (const KUrl&)), this, SLOT (urlChangedInCombo (const KUrl&)));

	connect (dir, SIGNAL (fileSelected (const KFileItem*)), this, SLOT (fileActivated (const KFileItem*)));

	setURL (QDir::currentPath ());
}

RKFileBrowserWidget::~RKFileBrowserWidget () {
	RK_TRACE (APP);
}

void RKFileBrowserWidget::setURL (const QString &url) {
	RK_TRACE (APP);

	urlbox->setUrl (url);
	dir->setUrl (url, true);
}

void RKFileBrowserWidget::urlChangedInView (const KUrl &url) {
	RK_TRACE (APP);

	urlbox->setUrl (url);
}

void RKFileBrowserWidget::urlChangedInCombo (const QString &url) {
	RK_TRACE (APP);

	dir->setUrl (url, true);
}

void RKFileBrowserWidget::urlChangedInCombo (const KUrl &url) {
	RK_TRACE (APP);

	dir->setUrl (url, true);
}

bool RKFileBrowserWidget::eventFilter (QObject *watched, QEvent *e) {
	// don't trace
/* KDE4: do we still need this?
	// fix size of popup (copied from katefileselector.cpp)
	Q3ListBox *lb = urlbox->listBox ();
	if (watched == lb && e->type() == QEvent::Show) {
		int add = lb->height() < lb->contentsHeight() ? lb->verticalScrollBar()->width() : 0;
		int w = qMin (topLevelWidget ()->width(), lb->contentsWidth() + add);
		lb->resize (w, lb->height());
	} */
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
			new KRun (item->url (), topLevelWidget(), item->mode (), item->isLocalFile ());
		}
	}
}

#include "rkfilebrowser.moc"
