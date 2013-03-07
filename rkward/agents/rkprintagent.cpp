/***************************************************************************
                          rkprintagent  -  description
                             -------------------
    begin                : Mon Aug 01 2011
    copyright            : (C) 2011 by Thomas Friedrichsmeier
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

#include "rkprintagent.h"

#include <QFile>
#include <QTimer>
#include <QDateTime>

#include <krun.h>
#include <kservice.h>
#include <kmessagebox.h>
#include <klocale.h>

#include "../rkward.h"

#include "../debug.h"

RKPrintAgent::RKPrintAgent () : QObject () {
	RK_TRACE (APP)
}

RKPrintAgent::~RKPrintAgent () {
	RK_TRACE (APP)

	if (delete_file) QFile (file).remove ();
	if (provider) provider->deleteLater ();
}

void RKPrintAgent::printPostscript (const QString &file, bool delete_file) {
	RK_TRACE (APP)

	KService::Ptr service = KService::serviceByDesktopPath ("okular_part.desktop");
	if (!service) service = KService::serviceByDesktopPath ("kpdf_part.desktop");

	KParts::ReadOnlyPart *provider = 0;
	if (service) provider = service->createInstance<KParts::ReadOnlyPart> (0);
	else RK_DEBUG (APP, DL_WARNING, "No KDE service found for postscript printing");

	QAction *printaction = 0;
	if (provider) {
		printaction = provider->action ("print");
		if (!printaction) printaction = provider->action ("file_print");
		if (!printaction) {
			QAction *a = new QAction (provider);
			bool ok = connect (a, SIGNAL (triggered()), provider, SLOT (slotPrint()));
			if (ok) printaction = a;
		}
		if (!(printaction && provider->openUrl (KUrl::fromLocalFile (file)))) {
			RK_DEBUG (APP, DL_WARNING, "No print action in postscript provider");
			delete provider;
			provider = 0;
		}
	}

	if (!provider) {
		RK_DEBUG (APP, DL_WARNING, "No valid postscript postscript provider was found");
		KMessageBox::sorry (RKWardMainWindow::getMain (), i18n ("No service was found to provide a KDE print dialog for postscript files. We will try to open a generic postscript viewer (if any), instead.<br><br>Consider installing 'okular', or configure RKWard not to attempt to print using a KDE print dialog."), i18n ("Unable to open KDE print dialog"));
		// fallback: If we can't find a proper part, try to invoke a standalone PS reader, instead
		KRun::runUrl (KUrl::fromLocalFile (file), "appication/postscript", RKWardMainWindow::getMain ());
		return;
	}

	RKPrintAgent *agent = new RKPrintAgent ();
	agent->file = file;
	agent->delete_file = delete_file;
	agent->provider = provider;

	// very hacky heuristic to try to find out, whether the print action is synchronous or asnchronous. If the latter, delete after half an hour. If the former delete after printing.
	QTime ts;
	ts.start ();
	printaction->trigger ();
	if (ts.elapsed () < 5000) {
		QTimer::singleShot (1800000, agent, SLOT (deleteLater ()));
	} else {
		agent->deleteLater ();
	}
}

#include "rkprintagent.moc"
