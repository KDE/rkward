//
// C++ Implementation: %{MODULE}
//
// Description:
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <khtmlview.h>
#include <khtml_part.h>

#include <klocale.h>
#include <kiconloader.h>

#include <qfile.h>

#include "rkhelpwindow.h"

RKHelpWindow::RKHelpWindow(QWidget *parent, const char *name)
 : KMdiChildView(parent, name)
{
	khtmlpart = new KHTMLPart (this);
	khtmlpart->begin();
	khtmlpart->write("<HTML><BODY><H1>Help</H1>"
		"<P>Type Ctrl+I in the command editor to get help.</P>"
		"</BODY></HTML>");
	khtmlpart->end();
	khtmlpart->view()->setIcon(SmallIcon("help"));
	khtmlpart->view()->setName("Help"); 
	khtmlpart->view()->setCaption(i18n("Help")); 
}


RKHelpWindow::~RKHelpWindow()
{
}


#include "rkhelpwindow.moc"



bool RKHelpWindow::openURL(KURL url)
{
	if (QFile::exists( url.path() )) {
		khtmlpart->openURL(url);
		return(true);
	}
	else{
		return (false);
	}
}
