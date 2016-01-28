/***************************************************************************
                          showedittextfileagent  -  description
                             -------------------
    begin                : Tue Sep 13 2005
    copyright            : (C) 2005-2016 by Thomas Friedrichsmeier
    email                : thomas.friedrichsmeier@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "showedittextfileagent.h"

#include <klocale.h>
#include <kmessagebox.h>
#include <KMessageWidget>

#include <qlabel.h>
#include <qlayout.h>
#include <qfile.h>
#include <QVBoxLayout>
#include <QDialog>

#include "../windows/rkcommandeditorwindow.h"
#include "../rbackend/rinterface.h"
#include "../rbackend/rkrbackendprotocol_frontend.h"
#include "../windows/rkworkplace.h"
#include "../rkglobals.h"
#include "../rkward.h"

#include "../debug.h"

ShowEditTextFileAgent::ShowEditTextFileAgent (RBackendRequest *request, const QString &text, const QString &caption) : QObject (RKWardMainWindow::getMain ()) {
	RK_TRACE (APP);

	ShowEditTextFileAgent::request = request;

	message = new KMessageWidget (0);
	message->setMessageType (KMessageWidget::Information);
	message->setText (QString ("<p><strong>%1<strong></p><p>%2</p>").arg (caption).arg (QString (text).replace ('\n', "<br/>")));
	message->setCloseButtonVisible (false);
	QAction *done_action = new QAction (QIcon::fromTheme ("dialog-ok"), i18n ("Done"), message);
	connect (done_action, &QAction::triggered, this, &QObject::deleteLater);
	message->addAction (done_action);

	RKWorkplace::mainWorkplace ()->addMessageWidget (message);
}


ShowEditTextFileAgent::~ShowEditTextFileAgent () {
	RK_TRACE (APP);

	RKRBackendProtocolFrontend::setRequestCompleted (request);
	message->deleteLater ();
}

// static
void ShowEditTextFileAgent::showEditFiles (RBackendRequest *request) {
	RK_TRACE (APP);
	if (!request) return;

	QStringList files = request->params["files"].toStringList ();
	QStringList titles = request->params["titles"].toStringList ();
	QString wtitle = request->params["wtitle"].toString ();
	bool prompt = request->params["prompt"].toBool ();
	int count = files.count ();
	RK_ASSERT (titles.count () == count);

	QStringList display_titles;
	for (int n = 0; n < count; ++n) {
		QString title;
		if (!titles[n].isEmpty ()) title = titles[n];
		else if (count > 1) title = files[n];
		if (!wtitle.isEmpty ()) {
			if (!title.isEmpty ()) title.prepend (": ");
			title.prepend (wtitle);
		}
		display_titles.append (title);
	}

	bool r_highlighting = false;
	bool read_only = true;
	bool delete_files = false;
	if (request->type == RBackendRequest::ShowFiles) {
		RK_ASSERT (!request->synchronous);

		if (prompt) KMessageBox::informationList (RKWardMainWindow::getMain (), i18n ("A command running in the R-engine wants you to see the following file(s):\n"), display_titles, i18n ("Showing file(s)"), "show_files");

		delete_files = request->params["delete"].toBool ();
		RKRBackendProtocolFrontend::setRequestCompleted (request);
	} else if (request->type == RBackendRequest::EditFiles) {
		if (prompt) {
			new ShowEditTextFileAgent (request, i18n ("A command running in the R-engine wants you to edit the following file(s). Please look at these files, edit them as appropriate, and save them. When done, press the \"Done\"-button, or close this dialog to resume.<ul><li>") + display_titles.join ("</li></li>") + "</li></ul>", i18n ("Edit file(s)"));
		} else {
			RKRBackendProtocolFrontend::setRequestCompleted (request);
		}

		r_highlighting = true;
		read_only = false;
	} else {
		RK_ASSERT (false);
	}

	// do this last, as it may produce error messages, if some of the files could not be opened.
	for (int n = 0; n < count; ++n) {
		RKWorkplace::mainWorkplace ()->openScriptEditor (QUrl::fromLocalFile (files[n]), QString (), r_highlighting, read_only, display_titles[n], delete_files);
	}
}
