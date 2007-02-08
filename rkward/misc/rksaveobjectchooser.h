/***************************************************************************
                          rksaveobjectchooser  -  description
                             -------------------
    begin                : Mon Nov 27 2006
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

#ifndef RKSAVEOBJECTCHOOSER_H
#define RKSAVEOBJECTCHOOSER_H

#include <qwidget.h>

class QLineEdit;
class QCheckBox;

/** Simple helper widget to select an R symbol name to write something to. */
class RKSaveObjectChooser : public QWidget {
	Q_OBJECT
public:
	RKSaveObjectChooser (QWidget *parent, const QString &initial, const QString &prompt = QString::null);
	~RKSaveObjectChooser ();

	QString validizedSelectedObjectName ();
	bool isOk () const;
	void setObjectName (const QString &name);
	void setBackgroundColor (const QColor &color);
public slots:
	void nameEditChanged (const QString &);
	void overwriteConfirmChanged (int);
signals:
	void okStatusChanged (bool);
	void changed ();
private:
	bool object_exists;
	bool prev_ok;

	QLineEdit *name_edit;
	QCheckBox *overwrite_confirm;
};

#endif
