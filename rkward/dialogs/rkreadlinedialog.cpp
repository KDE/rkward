/***************************************************************************
                          rkreadlinedialog  -  description
                             -------------------
    begin                : Fri Sep 15 2006
    copyright            : (C) 2006, 2007 by Thomas Friedrichsmeier
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
#include <QTextEdit>
#include <qlabel.h>
#include <qapplication.h>
#include <qdesktopwidget.h>
#include <QScrollBar>

#include <klocale.h>
#include <kvbox.h>
#include <kglobalsettings.h>

#include "../rbackend/rcommand.h"

#include "../debug.h"

QRect RKReadLineDialog::stored_geom;

RKReadLineDialog::RKReadLineDialog (QWidget *parent, const QString &caption, const QString &prompt, RCommand *command) : KDialog (parent) {
	RK_TRACE (DIALOGS);
	RK_ASSERT (command);

	setModal (true);
	setCaption (caption);
	setButtons (KDialog::Ok | KDialog::Cancel);

	KVBox *page = new KVBox ();
	setMainWidget (page);

	new QLabel (caption, page);

	int screen_width = qApp->desktop ()->width () - 2*marginHint() - 2*spacingHint ();		// TODO is this correct on xinerama?

	QString context = command->fullOutput ();
	if (!context.isEmpty ()) {
		new QLabel (i18n ("Context:"), page);

		QTextEdit *output = new QTextEdit (page);
		output->setUndoRedoEnabled (false);
		output->setPlainText (QString ());
		output->setCurrentFont (KGlobalSettings::fixedFont ());
		output->setLineWrapMode (QTextEdit::NoWrap);
		output->insert (context);
		output->setReadOnly (true);
		// there seems to be no easier way to get at the contents width...
		int cwidth = output->horizontalScrollBar ()->maximum () + output->width ();
		output->setMinimumWidth (screen_width < cwidth ? screen_width : cwidth);
		output->moveCursor (QTextCursor::End);
		output->setFocusPolicy (Qt::NoFocus);
	}

	QLabel *promptl = new QLabel (prompt, page);
	promptl->setWordWrap (true);

	input = new QLineEdit (QString (), page);
	input->setMinimumWidth (fontMetrics ().maxWidth ()*20);
	input->setFocus ();
}

RKReadLineDialog::~RKReadLineDialog () {
	RK_TRACE (DIALOGS);
}

//static
bool RKReadLineDialog::readLine (QWidget *parent, const QString &caption, const QString &prompt, RCommand *command, QString *result) {
	RK_TRACE (DIALOGS);
	RK_ASSERT (result);

	RKReadLineDialog *dialog = new RKReadLineDialog (parent, caption, prompt, command);
	if (!stored_geom.isNull ()) dialog->setGeometry (stored_geom);
	int res = dialog->exec ();
	*result = dialog->input->text ();
	stored_geom = dialog->frameGeometry ();
	delete dialog;

	return (res != QDialog::Rejected);
}
