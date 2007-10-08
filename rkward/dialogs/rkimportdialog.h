/***************************************************************************
                          rkimportdialog  -  description
                             -------------------
    begin                : Tue Jan 30 2007
    copyright            : (C) 2007 by Thomas Friedrichsmeier
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

#ifndef RKIMPORTDIALOG_H
#define RKIMPORTDIALOG_H

#include <kfiledialog.h>

#include <qstringlist.h>
#include <q3hbox.h>

class QComboBox;
class RKContextMap;
class RKImportDialogFormatSelector;

/** This dialog is designed to allow the user to select a file, and file format. After that a suitable plugin
is opened automatically to deal with this type of file . */
class RKImportDialog : public KFileDialog {
	Q_OBJECT
public:
/** constructor
@param context_id The id of the context containing the relevant plugins
@param parent Parent widget (dialog will be centered on this widget, if non-zeor */
	RKImportDialog (const QString &context_id, QWidget *parent);
/** dtor */
	~RKImportDialog ();
public slots:
/** The currently selected file extension filter was changed. Update the file format selection box accordingly. */
	void filterChanged ();
protected:
/** reimplemented to a) invoke the relevant plugin, b) trigger self-destruction of the dialog */
	void accept ();
/** reimplemented to trigger self-destruction of the dialog */
	void reject ();
private:
	int format_count;
	RKImportDialogFormatSelector *format_selector;
	QStringList format_labels;
	QStringList filters;
	QStringList component_ids;
	RKContextMap *context;
};

/** Internal helper class to RKImportDialog. Needed solely to work around a design flaw in the KFileDialog API */
class RKImportDialogFormatSelector : public Q3HBox {
friend class RKImportDialog;
private:
	RKImportDialogFormatSelector ();
	~RKImportDialogFormatSelector () {};

	QComboBox *combo;
};

#endif
