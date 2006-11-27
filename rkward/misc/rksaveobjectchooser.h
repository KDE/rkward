/***************************************************************************
                          rksaveobjectchooser  -  description
                             -------------------
    begin                : Mon Nov 27 2006
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

#ifndef RKSAVEOBJECTCHOOSER_H
#define RKSAVEOBJECTCHOOSER_H

#include <qwidget.h>

class QLineEdit;
class QCheckBox;
class QLabel;

/** Simple helper widget to select an R symbol name to write something to. */
class RKSaveObjectChooser : public QWidget {
	Q_OBJECT
public:
	RKSaveObjectChooser (QWidget *parent, bool allow_overwrite, const QString &initial, QString prompt = QString::null);
	~RKSaveObjectChooser ();

	QString validizedSelectedObjectName ();
	bool isOk () const;
public slots:
	void nameEditChanged (const QString &);
	void overwriteConfirmChanged (int checked);
signals:
	void okStatusChanged (bool);
private:
	bool allow_overwrite;
	bool object_exists;
	bool prev_ok;

	QLineEdit *name_edit;
	QCheckBox *overwrite_confirm;
	QLabel *inuse_label;
};

#endif
