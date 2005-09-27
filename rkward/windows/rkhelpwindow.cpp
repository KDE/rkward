/***************************************************************************
                          rkward.h  -  description
                             -------------------
    begin                : Tue Oct 29 20:06:08 CET 2002 
    copyright            : (C) 2002 by Thomas Friedrichsmeier 
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

#include <khtmlview.h>
#include <khtml_part.h>
#include <klibloader.h>
#include <klocale.h>
#include <kiconloader.h>

#include <qfile.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qtimer.h>

#include "../rkglobals.h"
#include "../rkward.h"
#include <kparts/partmanager.h>

#include "rkhelpwindow.h"


RKHelpWindow::RKHelpWindow (QWidget *parent) : KMdiChildView (parent) {
	scroll_position=0;
	
	khtmlpart = new KHTMLPart (this,0,0,0,KHTMLPart::BrowserViewGUI);
	khtmlpart->setSelectable (true);
	
	khtmlpart->widget ()->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
	QHBoxLayout *pLayout = new QHBoxLayout (this, 0, -1, "layout");
	pLayout->addWidget (khtmlpart->widget());

	// We have to connect this in order to allow browsing.
	connect (khtmlpart->browserExtension (), SIGNAL(openURLRequest (const KURL &, const KParts::URLArgs &)), this, SLOT (slotOpenURLRequest (const KURL &, const KParts::URLArgs &)));
	
	connect (khtmlpart, SIGNAL (completed ()), this, SLOT (loadDone ()));
	
}

RKHelpWindow::~RKHelpWindow()
{
}

bool RKHelpWindow::openURL(const KURL &url, bool update_caption) {
	currentURL=url;
	bool ok = QFile (url.path ()).exists ();

	if (!ok) return false;

	khtmlpart->openURL (url);
	if (update_caption) {
		setMDICaption (url.fileName ());
	}

	return true;	
}

void RKHelpWindow::showOutputEmptyMessage () {
	khtmlpart->begin();
	khtmlpart->write (i18n ("<HTML><BODY><H1>RKWard output</H1>\n<P>The output is empty.</P>\n</BODY></HTML>"));
	khtmlpart->end();
}

/*!
    \fn RKHelpWindow::slotOpenURLRequest(const KURL &, const KParts::URLArgs & )
    Called when the user clicks on a link
 */
void RKHelpWindow::slotOpenURLRequest(const KURL &url, const KParts::URLArgs & )
{
	openURL (url);
}


/*!
    \fn RKHelpWindow::refresh()

	Reload current page.
 */
bool RKHelpWindow::refresh()
{
	int pos = khtmlpart->view()->contentsY();
	scroll_position=pos;
	return openURL (currentURL, false);
}


/*!
    \fn RKHelpWindow::loadDone()
	This slot is called when the new page has finished loading.
 */
void RKHelpWindow::loadDone () {
	khtmlpart->view()->setContentsPos (0, scroll_position);
}


/*!
    \fn RKHelpWindow::scrollToBottom()
	Scrolls to the bottom of the page.
 */
void RKHelpWindow::scrollToBottom () {
	khtmlpart->view()->setContentsPos (0, khtmlpart->view ()->contentsHeight ());
}

#include "rkhelpwindow.moc"
