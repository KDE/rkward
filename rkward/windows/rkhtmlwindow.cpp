/***************************************************************************
                          rkhtmlwindow  -  description
                             -------------------
    begin                : Wed Oct 12 2005
    copyright            : (C) 2005 by Thomas Friedrichsmeier
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
#include "rkhtmlwindow.h"

#include <khtmlview.h>
#include <khtml_part.h>
#include <klibloader.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kparts/partmanager.h>

#include <qfile.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qtimer.h>

#include "../rkglobals.h"
#include "../rkward.h"
#include "../settings/rksettingsmodulelogfiles.h"
#include "../misc/rkcommonfunctions.h"
#include "../debug.h"

RKHTMLWindow::RKHTMLWindow (QWidget *parent) : KMdiChildView (parent) {
	RK_TRACE (APP);
	scroll_position=0;
	
	khtmlpart = new KHTMLPart (this, 0, 0, 0, KHTMLPart::BrowserViewGUI);
	khtmlpart->setSelectable (true);
	
	khtmlpart->widget ()->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
	QHBoxLayout *pLayout = new QHBoxLayout (this, 0, -1, "layout");
	pLayout->addWidget (khtmlpart->widget ());

	// We have to connect this in order to allow browsing.
	connect (khtmlpart->browserExtension (), SIGNAL(openURLRequest (const KURL &, const KParts::URLArgs &)), this, SLOT (slotOpenURLRequest (const KURL &, const KParts::URLArgs &)));
	
	connect (khtmlpart, SIGNAL (completed ()), this, SLOT (loadDone ()));
}

RKHTMLWindow::~RKHTMLWindow () {
	RK_TRACE (APP);
}

bool RKHTMLWindow::openURL (const KURL &url) {
	RK_TRACE (APP);
	bool ok = QFile (url.path ()).exists ();

	if (!ok) return false;

	khtmlpart->openURL (url);
	updateCaption (url);

	return true;	
}

void RKHTMLWindow::updateCaption (const KURL &url) {
	RK_TRACE (APP);
	setMDICaption (url.filename ());
}

void RKHTMLWindow::slotOpenURLRequest(const KURL &url, const KParts::URLArgs & ) {
	RK_TRACE (APP);
	openURL (url);
}

void RKHTMLWindow::refresh () {
	RK_TRACE (APP);
	scroll_position = khtmlpart->view ()->contentsY ();
	openURL (khtmlpart->url ());
}

void RKHTMLWindow::loadDone () {
	RK_TRACE (APP);
	khtmlpart->view()->setContentsPos (0, scroll_position);
}



//##################### BEGIN RKOutputWindow #####################
//static 
RKOutputWindow* RKOutputWindow::current_output = 0;

RKOutputWindow::RKOutputWindow (QWidget *parent) : RKHTMLWindow (parent), KXMLGUIClient () {
	RK_TRACE (APP);

	// strip down the khtmlpart's GUI. remove some stuff we definitely don't need.
	RKCommonFunctions::removeContainers (khtmlpart, QStringList::split (',', "tools,security,extraToolBar,saveBackground,saveFrame,kget_menu"), true);

	KInstance* instance = new KInstance ("rkward");
	setInstance (instance);
	setXMLFile ("rkoutputwindow.rc");

	khtmlpart->insertChildClient (this);

	setIcon (SmallIcon ("text_block"));
	setMDICaption (i18n ("Output"));
	RKGlobals::rkApp ()->addWindow (this);

	outputFlush = new KAction (i18n ("&Flush"), 0, 0, this, SLOT (flushOutput ()), actionCollection (), "output_flush");
	outputRefresh = new KAction (i18n ("&Refresh"), 0, 0, this, SLOT (refreshOutput ()), actionCollection (), "output_refresh");

	RKGlobals::rkApp ()->m_manager->addPart (khtmlpart);

	KAction *action = khtmlpart->action ("saveDocument");
	if (action) action->setText (i18n ("Save Output as HTML"));
}

RKOutputWindow::~RKOutputWindow () {
	RK_TRACE (APP);

	if (this == current_output) {
		current_output = 0;
	}
}

bool RKOutputWindow::openURL (const KURL &url) {
	RK_TRACE (APP);

	bool ok = RKHTMLWindow::openURL (url);
	if (!ok) {
		showOutputEmptyMessage ();
	}
	return ok;
}

void RKOutputWindow::updateCaption (const KURL &) {
}

void RKOutputWindow::refresh () {
	scroll_position = khtmlpart->view ()->contentsHeight ();
	openURL (khtmlpart->url ());
}

//static
void RKOutputWindow::refreshOutput (bool show, bool raise) {
	RK_TRACE (APP);

	if (current_output) {
		if (raise) {
			current_output->activate ();
		}
		current_output->refresh ();
	} else {
		if (show) {
			// getCurrentOutput creates an output window
			getCurrentOutput ();
		}
	}
}

//static
RKOutputWindow* RKOutputWindow::getCurrentOutput () {
	RK_TRACE (APP);
	
	if (!current_output) {
		current_output = new RKOutputWindow (RKGlobals::rkApp ());

		KURL url (RKSettingsModuleLogfiles::filesPath () + "/rk_out.html");
		current_output->openURL (url);
	}

	return current_output;
}

void RKOutputWindow::flushOutput () {
	RK_TRACE (APP);

	int res = KMessageBox::questionYesNo (this, i18n ("Do you really want to flush the ouput? It won't be possible to restore it."), i18n ("Flush output?"));
	if (res==KMessageBox::Yes) {
		QFile out_file (RKSettingsModuleLogfiles::filesPath () + "/rk_out.html");
		out_file.remove ();
		refreshOutput (false, false);
	}
}

void RKOutputWindow::refreshOutput () {
	RK_TRACE (APP);

	refreshOutput (true, true);
}

void RKOutputWindow::showOutputEmptyMessage () {
	khtmlpart->begin();
	khtmlpart->write (i18n ("<HTML><BODY><H1>RKWard output</H1>\n<P>The output is empty.</P>\n</BODY></HTML>"));
	khtmlpart->end();
}

//#################### END RKOutputWindow ##########################
//#################### BEGIN RKHelpWindow ##########################

RKHelpWindow::RKHelpWindow (QWidget *parent) : RKHTMLWindow (parent), KXMLGUIClient () {
	RK_TRACE (APP);

	// strip down the khtmlpart's GUI. remove some stuff we definitely don't need.
	RKCommonFunctions::removeContainers (khtmlpart, QStringList::split (',', "tools,security,extraToolBar,saveBackground,saveDocument,saveFrame,kget_menu"), true);

/*	KInstance* instance = new KInstance ("rkward");
	setInstance (instance);
	setXMLFile ("rkhelpwindow.rc");		// TODO: create .rc-file

	khtmlpart->insertChildClient (this); */

	setIcon (SmallIcon ("help"));
	setMDICaption (i18n ("R Help"));
	RKGlobals::rkApp ()->addWindow (this);

	RKGlobals::rkApp ()->m_manager->addPart (khtmlpart);
}

RKHelpWindow::~RKHelpWindow () {
	RK_TRACE (APP);
}

#include "rkhtmlwindow.moc"
