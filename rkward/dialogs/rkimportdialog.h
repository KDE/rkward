/*
rkimportdialog - This file is part of the RKWard project. Created: Tue Jan 30 2007
SPDX-FileCopyrightText: 2007 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKIMPORTDIALOG_H
#define RKIMPORTDIALOG_H

#include <qstringlist.h>
#include <QFileDialog>

class QComboBox;
class RKComponentGUIXML;

/** This dialog is designed to allow the user to select a file, and file format. After that a suitable plugin
is opened automatically to deal with this type of file . */
class RKImportDialog : public QFileDialog {
	Q_OBJECT
public:
/** constructor
@param context_id The id of the context containing the relevant plugins
@param parent Parent widget (dialog will be centered on this widget, if non-zero */
	RKImportDialog (const QString &context_id, QWidget *parent);
/** dtor */
	~RKImportDialog ();
protected:
/** reimplemented to a) invoke the relevant plugin, b) trigger self-destruction of the dialog */
	void accept () override;
/** reimplemented to trigger self-destruction of the dialog */
	void reject () override;
private:
	int format_count;
	QStringList filters;
	QStringList component_ids;
	RKComponentGUIXML *context;
};

#endif
