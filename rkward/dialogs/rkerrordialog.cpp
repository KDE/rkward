/***************************************************************************
                          rkerrordialog  -  description
                             -------------------
    begin                : Thu Apr 25 2013
    copyright            : (C) 2013 by Thomas Friedrichsmeier
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

#include "rkerrordialog.h"

#include <kdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kvbox.h>
#include <QLabel>
#include <QTextEdit>

#include "../rbackend/rinterface.h"
#include "../rbackend/rksessionvars.h"
#include "../misc/rkprogresscontrol.h"
#include "../rkglobals.h"
#include "../rkward.h"

#include "../debug.h"

void RKErrorDialog::reportableErrorMessage (QWidget* parent_widget, const QString& user_message, const QString &details, const QString& caption, const QString& message_code) {
	RK_TRACE (APP);

	if (!parent_widget) parent_widget = RKWardMainWindow::getMain ();
	// adjusted from KMessageBox::detailedError
	KDialog *dialog = new KDialog (parent_widget, Qt::Dialog);
	dialog->setCaption (caption);
	if (details.isEmpty ()) dialog->setButtons (KDialog::Ok | KDialog::No);
	else dialog->setButtons (KDialog::Ok | KDialog::No | KDialog::Details);
	dialog->setButtonText (KDialog::No, i18n ("Report As Bug"));
	dialog->setObjectName ("error");
	dialog->setDefaultButton (KDialog::Ok);
	dialog->setEscapeButton (KDialog::Ok);
	KMessageBox::Options options = KMessageBox::Notify | KMessageBox::AllowLink;
	dialog->setModal (true);

	int ret = KMessageBox::createKMessageBox (dialog, QMessageBox::Critical, user_message, QStringList(), QString(), 0, options, details);

	if (ret == KDialog::No) {
		reportBug (parent_widget, (message_code.isEmpty () ? QString () : i18n ("Message code: %1\n", message_code)) + user_message);
	}
}

void RKErrorDialog::reportBug (QWidget* parent_widget, const QString& message_info) {
	RK_TRACE (APP);

	if (!parent_widget) parent_widget = RKWardMainWindow::getMain ();

	QString report_template = i18n ("---Problem description---\n");
	if (message_info.isEmpty ()) {
		report_template.append (i18n ("Please give a brief summary on the problem:\n###Please fill in###\n\n"));
	} else {
		report_template.append (i18n ("I encountered the error message quoted below. Additionally, I saw the following symptoms:\n###Please fill in (if applicable)###\n\n"));
	}
	report_template.append (i18n ("What - in detail - did you do directly before you encountered this problem?\n###Please fill in###\n\n"));
	report_template.append (i18n ("When you try to repeat the above, does the problem occur again (no, sometimes, always)?\n###Please fill in###\n\n"));
	report_template.append (i18n ("If applicable: When doing the same thing in an R session outside of RKWard, do you see the same problem?\n###Please fill in###\n\n"));
	report_template.append (i18n ("Do you have any further information that might help us to track this problem down? In particular, if applicable, can you provide sample data and sample R code to reproduce this problem?\n###Please fill in###\n\n"));
	report_template.append (i18n ("RKWard is available in many different packagings, and sometimes problems are specific to one method of installation. How did you install RKWard (which file(s) did you download)?\n###Please fill in###\n\n"));

	if (!message_info.isEmpty ()) {
		report_template.append ("\n---Error Message---\n");
		report_template.append (message_info);
		report_template.append ("\n");
	}

	report_template.append ("\n---Session Info---\n");
	bool ok = false;
	if (!RKGlobals::rInterface ()->backendIsDead ()) {
		RCommand *command = new RCommand ("rk.sessionInfo()", RCommand::App);
		RKProgressControl *control = new RKProgressControl (parent_widget, i18n ("Please stand by while gathering some information on your setup.\nIn case the backend has died or hung up, you may want to press 'Cancel' to skip this step."), i18n ("Gathering setup information"), RKProgressControl::CancellableNoProgress);
		control->addRCommand (command, true);
		RKGlobals::rInterface ()->issueCommand (command);
		control->doModal (false);
		ok = command->succeeded ();
		report_template.append (control->fullCommandOutput ());
		delete control;
	}
	if (!ok) {
		report_template.append (RKSessionVars::frontendSessionInfo ().join ("\n"));
		report_template.append ("\n- backend not available or rk.sessionInfo() canceled -\n");
	}

	KDialog *dialog = new KDialog (parent_widget);
	QObject::connect (dialog, SIGNAL (finished(int)), dialog, SLOT (deleteLater()));
	dialog->setCaption (i18n ("Reporting bugs in RKWard"));
	dialog->setButtons (KDialog::Ok);
	KVBox *vbox = new KVBox (dialog);
	dialog->setMainWidget (vbox);
	QLabel *label = new QLabel (i18n ("<p><b>Where should I report bugs or wishes?</b></p><p>Please submit your bug reports or wishes at <a href=\"%1\">%1</a> or send email to <a href=\"mailto:%2\">%2</a>.</p>"
							"<p><b>What information should I provide?</b></p><p>Please copy the information shown below, and fill in the details to the questions.</p>"
							, QString ("http://p.sf.net/rkward/bugs"), QString ("rkward-devel@lists.sourceforge.net")), vbox);
	label->setWordWrap (true);
	label->setOpenExternalLinks (true);
	QTextEdit *details = new QTextEdit (vbox);
	details->setReadOnly (true);
	details->setText (report_template);

	dialog->show ();

}
