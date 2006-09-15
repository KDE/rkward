/***************************************************************************
                          rkreadlinedialog  -  description
                             -------------------
    begin                : Fri Sep 15 2006
    copyright            : (C) 2006 by Thomas Friedrichsmeier
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

#include "rkreadlinedialog.h"

#include <qlineedit.h>
#include <qtextedit.h>
#include <qlabel.h>
#include <qvbox.h>

#include <klocale.h>

#include "../rbackend/rcommand.h"

#include "../debug.h"

RKReadLineDialog::RKReadLineDialog (QWidget *parent, const QString &caption, const QString &prompt, RCommand *command) : KDialogBase (parent, 0, true, caption, KDialogBase::Ok | KDialogBase::Cancel) {
	RK_TRACE (DIALOGS);

	QVBox *page = makeVBoxMainWidget ();
	new QLabel (caption, page);

	QString context = command->fullOutput ();
	if (!context.isEmpty ()) {
		new QLabel (i18n ("Context:"), page);

		QTextEdit *output = new QTextEdit (page);
		output->setUndoRedoEnabled (false);
		output->setTextFormat (QTextEdit::PlainText);
		output->setCurrentFont (QFont ("Courier"));
		output->setWordWrap (QTextEdit::NoWrap);
		output->setText (context);
		output->setReadOnly (true);
		setMinimumWidth (output->contentsWidth ());		// TODO: should never be wider than screen
		output->scrollToBottom ();
	}

	new QLabel (prompt, page);

	input = new QLineEdit (QString (), page);
	input->setMinimumWidth (fontMetrics ().maxWidth ()*20);
}

RKReadLineDialog::~RKReadLineDialog () {
	RK_TRACE (DIALOGS);
}

//static
bool RKReadLineDialog::readLine (QWidget *parent, const QString &caption, const QString &prompt, RCommand *command, QString *result) {
	RK_TRACE (DIALOGS);
	RK_ASSERT (result);

	RKReadLineDialog *dialog = new RKReadLineDialog (parent, caption, prompt, command);
	int res = dialog->exec ();
	*result = dialog->input->text ();

	if (res == KDialogBase::Cancel) return false;
	return true;
}
