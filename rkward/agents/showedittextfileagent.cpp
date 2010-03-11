/***************************************************************************
                          showedittextfileagent  -  description
                             -------------------
    begin                : Tue Sep 13 2005
    copyright            : (C) 2005, 2009, 2010 by Thomas Friedrichsmeier
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

#include "showedittextfileagent.h"

#include <klocale.h>
#include <kdialog.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qfile.h>
#include <QVBoxLayout>

#include "../windows/rkcommandeditorwindow.h"
#include "../rbackend/rinterface.h"
#include "../rbackend/rembedinternal.h"
#include "../windows/rkworkplace.h"
#include "../rkglobals.h"
#include "../rkward.h"

#include "../debug.h"

ShowEditTextFileAgent::ShowEditTextFileAgent (RCallbackArgs *args, const QString &text, const QString &caption) : QObject (RKWardMainWindow::getMain ()) {
	RK_TRACE (APP);

	ShowEditTextFileAgent::args = args;

	dialog = new KDialog (0);

	// dialog setup
	dialog->setCaption (caption);
	dialog->setButtons (KDialog::Ok);
	dialog->setModal (false);

	QWidget *page = new QWidget (dialog);
	dialog->setMainWidget (page);
	QVBoxLayout *layout = new QVBoxLayout (page);
	layout->setContentsMargins (0, 0, 0, 0);
	QLabel *label = new QLabel (text, page);
	label->setWordWrap (true);
	layout->addWidget (label);

	dialog->setButtonText (KDialog::Ok, i18n ("Done"));

	connect (dialog, SIGNAL (finished ()), this, SLOT (done ()));

	// do it
	dialog->show ();
}


ShowEditTextFileAgent::~ShowEditTextFileAgent () {
	RK_TRACE (APP);
}

// static
void ShowEditTextFileAgent::showEditFiles (RCallbackArgs *args) {
	RK_TRACE (APP);
	if (!args) return;

	QString caption;
	QString message;
	QString message_snip1 = i18n (" For that reason processing has been stopped for now. Press the \"Done\"-button, or close this dialog once you think it is safe to resume.\n\n");
	QString message_snip2 = i18n ("The file(s) have been opened in text-windows. The following is a list of the file(s) in question:\n\n");

	QStringList files = args->params["files"].toStringList ();
	QStringList titles = args->params["titles"].toStringList ();
	QString wtitle = args->params["wtitle"].toString ();
	int count = files.count ();
	RK_ASSERT (titles.count () == count);

	QString bad_files_list;
	bool r_highlighting = false;
	bool read_only = true;
	if (args->type == RCallbackArgs::RShowFiles) {
		caption = i18n ("Showing file(s)");
		message = i18n ("A command running in the R-engine wants you to see one or more file(s). RKWard can not determine, whether it is safe to continue processing R commands, before you have read the file(s) in question.") + message_snip1 + message_snip2;
	} else if (args->type == RCallbackArgs::REditFiles) {
		caption = i18n ("Edit file(s)");
		message = i18n ("A command running in the R-engine wants you to edit one or more file(s). RKWard can not determine, whether it is safe to continue processing R commands, before you have read/edited (and saved) the file(s) in question.") + message_snip1 + message_snip2;

		r_highlighting = true;
		read_only = false;
	}

	for (int n = 0; n < count; ++n) {
		QString title;
		if (!titles[n].isEmpty ()) title = titles[n];
		else if (count > 1) title = files[n];
		if (!wtitle.isEmpty ()) {
			if (!title.isEmpty ()) title.prepend (": ");
			title.prepend (wtitle);
		}

		message.append (title + "\n");

		bool ok = RKWorkplace::mainWorkplace ()->openScriptEditor (KUrl::fromLocalFile (files[n]), QString (), r_highlighting, read_only, title);

		if (!ok)  {
			bad_files_list.append ("- ").append (title).append (" (").append (files[n]).append (")\n");
		}
	}

	if (!bad_files_list.isEmpty ()) {
		message.append (i18n ("\n\nThe following of the above files were not readable and have not been opened:\n\n"));
		message.append (bad_files_list);
	}

	new ShowEditTextFileAgent (args, message, caption);
}

void ShowEditTextFileAgent::done () {
	RK_TRACE (APP);
	dialog->deleteLater ();

	QStringList files = args->params["files"].toStringList ();
	if ((args->type == RCallbackArgs::RShowFiles) && args->params["delete"].toBool ()) {
		for (int n = 0; n < files.count (); ++n) {
			RK_ASSERT (QFile::remove (QString (files[n])));
		}
	}

	MUTEX_LOCK;
	// this line is what causes the backend-thread to resume processing:
	args->done = true;
	MUTEX_UNLOCK;

	deleteLater ();
}

#include "showedittextfileagent.moc"
