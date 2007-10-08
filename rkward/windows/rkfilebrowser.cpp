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
#include <q3vbox.h>
#include <q3listbox.h>
//Added by qt3to4:
#include <QEvent>
#include <Q3VBoxLayout>

#include "rkworkplace.h"
#include "../rkward.h"
#include "../misc/rkdummypart.h"

#include "../debug.h"

// static
RKFileBrowser *RKFileBrowser::main_browser = 0;

RKFileBrowser::RKFileBrowser (QWidget *parent, bool tool_window, const char *name) : RKMDIWindow (parent, FileBrowserWindow, tool_window, name) {
	RK_TRACE (APP);

	real_widget = 0;

	Q3VBoxLayout *layout = new Q3VBoxLayout (this);
	layout_widget = new Q3VBox (this);
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



RKFileBrowserWidget::RKFileBrowserWidget (QWidget *parent) : Q3VBox (parent) {
	RK_TRACE (APP);

	KToolBar *toolbar = new KToolBar (this);
	toolbar->setIconSize (16);

	urlbox = new KURLComboBox (KURLComboBox::Directories, true, this);
	KURLCompletion* cmpl = new KURLCompletion (KURLCompletion::DirCompletion);
	urlbox->setCompletionObject (cmpl);
	urlbox->setAutoDeleteCompletionObject (true);
	urlbox->setSizePolicy (QSizePolicy (QSizePolicy::Expanding, QSizePolicy::Fixed));
	urlbox->listBox ()->installEventFilter (this);

	dir = new KDirOperator (KURL (), this);
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

	setURL (QDir::currentPath ());
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
	Q3ListBox *lb = urlbox->listBox ();
	if (watched == lb && e->type() == QEvent::Show) {
		int add = lb->height() < lb->contentsHeight() ? lb->verticalScrollBar()->width() : 0;
		int w = qMin (topLevelWidget ()->width(), lb->contentsWidth() + add);
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
