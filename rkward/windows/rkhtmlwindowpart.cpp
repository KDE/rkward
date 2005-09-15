/***************************************************************************
                          rkhtmlwindowpart  -  description
                             -------------------
    begin                : Thu Sep 15 2005
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

#include "rkhtmlwindowpart.h"

#include <kmdichildview.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <klocale.h>
#include <khtml_part.h>

#include <qfile.h>

#include "rkhelpwindow.h"
#include "../settings/rksettingsmodulelogfiles.h"
#include "../rkward.h"
#include "../rkglobals.h"
#include "../debug.h"

//static 
RKHTMLWindowPart* RKHTMLWindowPart::current_output = 0;

RKHTMLWindowPart::RKHTMLWindowPart (bool is_output) : KParts::Part (RKGlobals::rkApp ()) {
	RK_TRACE (APP);

	KInstance* instance = new KInstance ("rkward");
	setInstance (instance);
	setXMLFile ("rkhtmlwindowpart.rc");

	widget = new RKHelpWindow (RKGlobals::rkApp ());

// TODO: this is ugly as the night. Eventually, we'll have to split this class into an RKHelpWindowPart, and an RKOutputWindowPart
	if (is_output) {
		widget->khtmlpart->insertChildClient (this);
	}

	if (is_output) {
		getWidget ()->setIcon (SmallIcon ("text_block"));
		getWidget ()->setMDICaption (i18n ("Output"));
		RKGlobals::rkApp ()->addWindow (getWidget ());
	} else {
		getWidget ()->setIcon (SmallIcon ("help"));
		RKGlobals::rkApp ()->addWindow (getWidget ());
	}

	outputFlush = new KAction (i18n ("&Flush"), 0, 0, this, SLOT (flushOutput ()), actionCollection (), "output_flush");
	outputRefresh = new KAction (i18n ("&Refresh"), 0, 0, this, SLOT (refreshOutput ()), actionCollection (), "output_refresh");

	RKGlobals::rkApp ()->m_manager->addPart (widget->khtmlpart);
}

RKHTMLWindowPart::~RKHTMLWindowPart () {
	RK_TRACE (APP);

	if (this == current_output) {
		current_output = 0;
	}
}

// this could really be inlined. By putting it here, however, we save the #include "rkhelpwindow.h" in the header.
KMdiChildView* RKHTMLWindowPart::getWidget () {
	RK_TRACE (APP);

	return widget;
}

//static
void RKHTMLWindowPart::openHTML (const KURL &url, bool is_output) {
	RK_TRACE (APP);

	RKHTMLWindowPart* part;

	if (!is_output) {
		part = new RKHTMLWindowPart (is_output);
		part->widget->openURL (url);
	} else {
		part = getCurrentOutput ();
		if (!part->widget->openURL (url, false)) {
			part->widget->showOutputEmptyMessage ();
		}
	}
}

//static
void RKHTMLWindowPart::refreshOutput (bool show, bool raise) {
	RK_TRACE (APP);

	if (current_output) {
		if (raise) {
			current_output->widget->raise ();
			current_output->widget->refresh ();
		}
	} else {
		if (show) {
			// getCurrentOutput creates an output window
			getCurrentOutput ();
		}
	}
}

//static
RKHTMLWindowPart* RKHTMLWindowPart::getCurrentOutput () {
	RK_TRACE (APP);
	
	if (!current_output) {
		current_output = new RKHTMLWindowPart (true);

		KURL url (RKSettingsModuleLogfiles::filesPath () + "/rk_out.html");
		current_output->openHTML (url, true);
	}

	return current_output;
}

void RKHTMLWindowPart::flushOutput () {
	RK_TRACE (APP);

	int res = KMessageBox::questionYesNo (getCurrentOutput ()->getWidget (), i18n ("Do you really want to flush the ouput? It won't be possible to restore it."), i18n ("Flush output?"));
	if (res==KMessageBox::Yes) {
		QFile out_file (RKSettingsModuleLogfiles::filesPath () + "/rk_out.html");
		out_file.remove ();
		refreshOutput (false, false);
	}
}

void RKHTMLWindowPart::refreshOutput () {
	RK_TRACE (APP);

	refreshOutput (true, true);
}

#include "rkhtmlwindowpart.moc"
