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

#include "../rkglobals.h"
#include "../rkward.h"
#include <kparts/partmanager.h>

#include "rkhelpwindow.h"


RKHelpWindow::RKHelpWindow(QWidget *parent, const char *name)
 : KMdiChildView(parent, name)
{
	khtmlpart = new KHTMLPart(this,0,0,0,KHTMLPart::BrowserViewGUI);
	khtmlpart->setSelectable(true);
	khtmlpart->view()->setIcon(SmallIcon("help"));
	khtmlpart->view()->setName("Help"); 
	khtmlpart->view()->setCaption(i18n("Help")); 

	(RKGlobals::rkApp()->m_manager)->addPart(khtmlpart,false);
	khtmlpart->widget()->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
	pLayout = new QHBoxLayout( this, 0, -1, "layout");
	pLayout->addWidget(khtmlpart->widget());

	// We have to connect this in order to allow browsing.
	connect( khtmlpart->browserExtension(), SIGNAL( openURLRequest( const KURL &, const KParts::URLArgs & ) ), this, SLOT( slotOpenURLRequest(const KURL &, const KParts::URLArgs & ) ) );
	
}


RKHelpWindow::~RKHelpWindow()
{
}


#include "rkhelpwindow.moc"



bool RKHelpWindow::openURL(KURL url)
{
	if (QFile::exists( url.path() )) {
		khtmlpart->openURL(url);
		setTabCaption(url.fileName());
		setCaption(url.prettyURL());
		return(true);
	}
	else{
		return (false);
	}
}


/*!
    \fn RKHelpWindow::slotOpenURLRequest(const KURL &, const KParts::URLArgs & )
    Called when the user clicks on a link
 */
void RKHelpWindow::slotOpenURLRequest(const KURL &url, const KParts::URLArgs & )
{
	khtmlpart->openURL(url);
	setTabCaption(url.fileName());
	setCaption(url.prettyURL());
}
