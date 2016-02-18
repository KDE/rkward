/***************************************************************************
                          rkreadlinedialog  -  description
                             -------------------
    begin                : Fri Sep 15 2006
    copyright            : (C) 2006, 2007, 2008, 2009 by Thomas Friedrichsmeier
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

#include "rkreadlinedialog.h"

#include <qlineedit.h>
#include <QTextEdit>
#include <qlabel.h>
#include <qapplication.h>
#include <qdesktopwidget.h>
#include <QScrollBar>
#include <QTimer>
#include <QVBoxLayout>
#include <QFontDatabase>

#include <KLocalizedString>
#include <kglobalsettings.h>

#include "../rbackend/rcommand.h"
#include "../misc/rkdialogbuttonbox.h"

#include "../debug.h"

QByteArray RKReadLineDialog::stored_geom;

RKReadLineDialog::RKReadLineDialog (QWidget *parent, const QString &caption, const QString &prompt, RCommand *command) : QDialog (parent) {
	RK_TRACE (DIALOGS);
	RK_ASSERT (command);

	setModal (true);
	setWindowTitle (caption);
	QVBoxLayout *layout = new QVBoxLayout (this);

	layout->addWidget (new QLabel (caption, this));

	int screen_width = qApp->desktop ()->availableGeometry (this).width ();

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

	QLabel *promptl = new QLabel (prompt, this);
	promptl->setWordWrap (true);
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
	QTimer::singleShot (0, dialog->input, SLOT(setFocus()));
	int res = dialog->exec ();
	*result = dialog->input->text ();
	stored_geom = dialog->saveGeometry ();
	delete dialog;

	return (res != QDialog::Rejected);
}
