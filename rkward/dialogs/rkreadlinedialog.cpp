/*
rkreadlinedialog - This file is part of RKWard (https://rkward.kde.org). Created: Fri Sep 15 2006
SPDX-FileCopyrightText: 2006-2009 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkreadlinedialog.h"

#include <QApplication>
#include <QScreen>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QScrollBar>
#include <QTimer>
#include <QVBoxLayout>
#include <QFontDatabase>

#include <KLocalizedString>

#include "../rbackend/rcommand.h"
#include "../misc/rkdialogbuttonbox.h"
#include "../misc/rkcommonfunctions.h"

#include "../debug.h"

QByteArray RKReadLineDialog::stored_geom;

RKReadLineDialog::RKReadLineDialog (QWidget *parent, const QString &caption, const QString &prompt, RCommand *command) : QDialog (parent) {
	RK_TRACE (DIALOGS);
	RK_ASSERT (command);

	setModal (true);
	setWindowTitle (caption);
	QVBoxLayout *layout = new QVBoxLayout (this);

	layout->addWidget (new QLabel (caption, this));

	int screen_width = screen() ? screen()->availableGeometry().width() : QApplication::primaryScreen()->availableGeometry().width();

	QString context = command->fullOutput ();
	if (!context.isEmpty ()) {
		layout->addWidget (new QLabel (i18n ("Context:"), this));

		QTextEdit *output = new QTextEdit (this);
		output->setUndoRedoEnabled (false);
		output->setPlainText (QString ());
		output->setCurrentFont (QFontDatabase::systemFont (QFontDatabase::FixedFont));
		output->setLineWrapMode (QTextEdit::NoWrap);
		output->insertPlainText (context);
		output->setReadOnly (true);
		// there seems to be no easier way to get at the contents width...
		int cwidth = output->horizontalScrollBar ()->maximum () + output->width ();
		output->setMinimumWidth (screen_width < cwidth ? screen_width : cwidth);
		output->moveCursor (QTextCursor::End);
		output->setFocusPolicy (Qt::NoFocus);
		layout->addWidget (output);
		layout->setStretchFactor (output, 10);
	}

	QLabel *promptl = RKCommonFunctions::wordWrappedLabel (prompt);
	layout->addWidget (promptl);

	input = new QLineEdit (QString (), this);
	input->setMinimumWidth (fontMetrics ().maxWidth ()*20);
	input->setFocus ();
	layout->addWidget (input);

	RKDialogButtonBox *box = new RKDialogButtonBox (QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	layout->addWidget (box);
}

RKReadLineDialog::~RKReadLineDialog () {
	RK_TRACE (DIALOGS);
}

//static
bool RKReadLineDialog::readLine (QWidget *parent, const QString &caption, const QString &prompt, RCommand *command, QString *result) {
	RK_TRACE (DIALOGS);
	RK_ASSERT (result);

	RKReadLineDialog *dialog = new RKReadLineDialog (parent, caption, prompt, command);
	if (!stored_geom.isNull ()) dialog->restoreGeometry (stored_geom);
	QTimer::singleShot(0, dialog, [dialog](){ dialog->input->setFocus(); });
	int res = dialog->exec ();
	*result = dialog->input->text ();
	stored_geom = dialog->saveGeometry ();
	delete dialog;

	return (res != QDialog::Rejected);
}
