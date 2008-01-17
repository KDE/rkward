/***************************************************************************
                          rkfilebrowser  -  description
                             -------------------
    begin                : Thu Apr 26 2007
    copyright            : (C) 2007, 2008 by Thomas Friedrichsmeier
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
#include <kcompletionbox.h>
#include <ktoolbar.h>
#include <krun.h>
#include <kactioncollection.h>

#include <qdir.h>
#include <qlayout.h>
#include <QEvent>
#include <QVBoxLayout>
#include <QScrollBar>

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
		setFocusProxy (real_widget);
	}

	RKMDIWindow::showEvent (e);
}

void RKFileBrowser::currentWDChanged () {
	RK_TRACE (APP);
}

/////////////////// RKFileBrowserWidget ////////////////////

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
	urlbox->completionBox (true)->installEventFilter (this);
	setFocusProxy (urlbox);

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

	connect (dir, SIGNAL (fileSelected (const KFileItem&)), this, SLOT (fileActivated (const KFileItem&)));

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

	return (KVBox::eventFilter (o, e));
}

void RKFileBrowserWidget::fileActivated (const KFileItem& item) {
	RK_TRACE (APP);

	QString mimetype = item.mimetype ();
	if (mimetype.startsWith ("text/")) {
		if (mimetype == "text/html") {
			RKWorkplace::mainWorkplace ()->openHelpWindow (item.url (), true);
		} else {
			RKWorkplace::mainWorkplace ()->openScriptEditor (item.url ());
		}
	} else {
		if (item.name (true).endsWith (".rdata")) {
			RKWardMainWindow::getMain ()->fileOpenAskSave (item.url ());
		} else {
			new KRun (item.url (), topLevelWidget(), item.mode (), item.isLocalFile ());
		}
	}
}

#include "rkfilebrowser.moc"
