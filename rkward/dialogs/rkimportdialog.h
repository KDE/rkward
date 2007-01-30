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

class QHBox;
class QComboBox;
class RKContextMap;

class RKImportDialog : public KFileDialog {
	Q_OBJECT
public:
	RKImportDialog (const QString &context_id, QWidget *parent);
	~RKImportDialog ();
public slots:
	void filterChanged ();
protected:
	void slotOk ();
	void slotCancel ();
private:
	int format_count;
	QHBox *format_selection_box;
	QComboBox *format_combo;
	QStringList format_labels;
	QStringList filters;
	QStringList component_ids;
	RKContextMap *context;
};

#endif
