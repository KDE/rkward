/*
rkerrordialog - This file is part of RKWard (https://rkward.kde.org). Created: Thu Apr 25 2013
SPDX-FileCopyrightText: 2013-2020 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkerrordialog.h"

#include <KLocalizedString>
#include <kmessagebox.h>

#include <QIcon>
#include <QLabel>
#include <QTextEdit>
#include <QTemporaryFile>
#include <QDir>
#include <QTextStream>
#include <QPushButton>
#include <QDialog>
#include <QVBoxLayout>
#include <QDesktopServices>

#include "../rbackend/rkrinterface.h"
#include "../rbackend/rksessionvars.h"
#include "../misc/rkprogresscontrol.h"
#include "../misc/rkdialogbuttonbox.h"
#include "../misc/rkcommonfunctions.h"

#include "../rkward.h"
#include "../version.h"

#include "../debug.h"

#define SUBMIT_ADDRESS "https://bugs.kde.org/enter_bug.cgi"

class RKBugzillaReportDialog : public QDialog {
public:
	RKBugzillaReportDialog(QWidget* parent, const QString& report_template) : QDialog(parent), report_template(report_template) {
		RK_TRACE (DIALOGS);

		setWindowTitle (i18n ("Reporting bugs in RKWard"));
		QVBoxLayout *layout = new QVBoxLayout (this);
		QLabel *label = RKCommonFunctions::wordWrappedLabel (i18n ("<p><b>Where should I report bugs or wishes?</b></p><p>Thank you for taking the time to help improve RKWard. To help us "
		                                  "handle your request, efficiently, please submit your bug reports or wishes in the "
		                                  "<a href=\"%1\">KDE bugtracking system</a>. Note that you need a user account for this, so that we will be able to contact you, "
		                                  "for follow-up questions. <b>If you do not have an account, please <a href=\"%2\">create one</a>, first.</b></p>"
		                                  "<p>In case this is not possible for some reason, refer to <a href=\"%3\">%3</a> for alternative ways of reporting issues.</p>",
		                         QString ("https://bugs.kde.org"), QString ("https://bugs.kde.org/createaccount.cgi"), QString ("https://rkward.kde.org/Bugs.html"))
		                          + i18n ("<p><b>What information should I provide, and how?</b></p>Clicking \"Report issue\" will take you to the "
		                                  "KDE bugtracking system. After logging in, some information will already be pre-filled into the report form. Please make sure "
		                                  "to fill in the missing bits - in English - where indicated, especially in the \"Comment\" field.</p>"));
		label->setOpenExternalLinks (true);
		layout->addWidget (label);

		RKDialogButtonBox *buttons = new RKDialogButtonBox (QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
		buttons->button (QDialogButtonBox::Ok)->setText (i18n ("Report issue"));
		buttons->button (QDialogButtonBox::Ok)->setIcon (QIcon::fromTheme("tools-report-bug"));
		layout->addWidget (buttons);

		connect (this, &QDialog::finished, this, &RKBugzillaReportDialog::deleteLater);
	}

	void accept () override {
		// The report template is just too large to pass it via GET, so we use a local proxy page to pass it in a POST request
		QTemporaryFile proxy;
		proxy.setFileTemplate (QDir::tempPath () + "/rkwardbugXXXXXX.html"); // Force .html-suffix, as it appears to be required on Windows
		proxy.setAutoRemove (false);
		proxy.open ();
		QTextStream out (&proxy);
		out << "<html><head><title>Relaying to " SUBMIT_ADDRESS "</title></head><body onLoad=\"document.getElementById('form').submit();\">\n";
		out << "<h1>" + i18n ("Forwarding you to the KDE bugtracking system") + "</h1>\n";
		out << "<p>" + i18n ("You are now being forwarded to the KDE bugtracking system. Should you continue to see this page for more than a few seconds (e.g. if JavaScript is disabled), please click \"Proceed\", below.") + "</p>\n";
		out << "<form name=\"form\" id=\"form\" action=\"" SUBMIT_ADDRESS "\" method=\"POST\">\n";
		out << "<input name=\"product\" type=\"hidden\" value=\"rkward\"/>\n";
		out << "<input name=\"component\" type=\"hidden\" value=\"general\"/>\n";
		out << "<input name=\"version\" type=\"hidden\" value=\"" RKWARD_VERSION "\"/>\n";
		out << "<input name=\"comment\" type=\"hidden\" value=\"" << report_template.toHtmlEscaped () << "\"/>\n";
		out << "<input type=\"submit\" value=\"" << i18n ("Proceed") << "\"/>\n";
		out << "</form></body></html>";
		proxy.close ();

		QDesktopServices::openUrl (QUrl::fromLocalFile (proxy.fileName ()));
		QDialog::accept ();
	}
private:
	QString report_template;
};

void RKErrorDialog::reportableErrorMessage (QWidget* parent_widget, const QString& user_message, const QString &details, const QString& caption, const QString& message_code) {
	RK_TRACE (APP);
	if (RKWardMainWindow::suppressModalDialogsForTesting()) return;

	if (!parent_widget) parent_widget = RKWardMainWindow::getMain ();
	// adjusted from KMessageBox::detailedError
	QDialog *dialog = new QDialog (parent_widget, Qt::Dialog);
	dialog->setWindowTitle (caption);
	QDialogButtonBox *buttonbox = new QDialogButtonBox (dialog);
	if (details.isEmpty ()) buttonbox->setStandardButtons (QDialogButtonBox::Ok | QDialogButtonBox::No);
	else {
		buttonbox->setStandardButtons (QDialogButtonBox::Ok | QDialogButtonBox::No | QDialogButtonBox::Help);
		buttonbox->button (QDialogButtonBox::Help)->setText (i18n ("Show Details"));
	}
	buttonbox->button (QDialogButtonBox::No)->setText (i18n ("Report As Bug"));
	dialog->setObjectName ("error");
	buttonbox->button (QDialogButtonBox::Ok)->setDefault (true);
	buttonbox->button (QDialogButtonBox::Ok)->setShortcut (Qt::Key_Escape);
	KMessageBox::Options options = KMessageBox::Notify | KMessageBox::AllowLink;
	dialog->setModal (true);

	int ret = KMessageBox::createKMessageBox(dialog, buttonbox, QMessageBox::Critical, user_message, QStringList(), QString(), nullptr, options, details);

	if (ret == QDialogButtonBox::No) {
		reportBug (parent_widget, (message_code.isEmpty () ? QString () : i18n ("Message code: %1\n", message_code)) + user_message);
	}
}

void RKErrorDialog::reportBug (QWidget* parent_widget, const QString& message_info) {
	RK_TRACE (APP);

	if (!parent_widget) parent_widget = RKWardMainWindow::getMain ();

	QString report_template = i18n ("---Problem description---\nPlease fill in the missing bits *in English*.\n\n");
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
	if (!RInterface::instance()->backendIsDead ()) {
		RCommand *command = new RCommand ("rk.sessionInfo()", RCommand::App);
		RKProgressControl *control = new RKProgressControl (parent_widget, i18n ("Please stand by while gathering some information on your setup.\nIn case the backend has died or hung up, you may want to press 'Cancel' to skip this step."), i18n ("Gathering setup information"), RKProgressControl::CancellableNoProgress);
		control->addRCommand (command, true);
		RInterface::issueCommand(command);
		ok = control->doModal (false);
		// NOTE: command is already deleted at this point
		report_template.append (control->fullCommandOutput ());
		delete control;
	}
	if (!ok) {
		report_template.append (RKSessionVars::frontendSessionInfo ().join ("\n"));
		report_template.append ("\n- backend not available or rk.sessionInfo() canceled -\n");
	}

	RKBugzillaReportDialog *dialog = new RKBugzillaReportDialog (parent_widget, report_template);
	dialog->show ();
}
