/***************************************************************************
                          getfilenamewidget  -  description
                             -------------------
    begin                : Tue Aug 24 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
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
#ifndef GETFILENAMEWIDGET_H
#define GETFILENAMEWIDGET_H

#include <qwidget.h>
#include <qstring.h>

class QLineEdit;
class QPushButton;

/**
Simple convenience class used to get a file/directoryname from the user. Basically provides a line-edit and a "browse"-button.

@author Thomas Friedrichsmeier
*/
class GetFileNameWidget : public QWidget {
	Q_OBJECT
public:
	enum FileType { ExistingFile=0, ExistingDirectory=1 };
    
	GetFileNameWidget (QWidget *parent, FileType mode, const QString &label, const QString &caption, const QString &initial);
	~GetFileNameWidget ();

/** set filename pattern filter, e.g. "*.cpp *.cc *.C|C++ Source Files\n*.h *.H|Header files" */
	void setFilter (const QString &filter) { _filter = filter; };
/** set the filename/location from outside */
	void setLocation (const QString &new_location);

/** retrieves the current location */
	QString getLocation ();
public slots:
	void locationEditChanged (const QString &);
	void browseButtonClicked ();
signals:
	void locationChanged ();
private:
	QLineEdit *location_edit;
	QPushButton *browse_button;
	QString caption;
	QString _filter;
	FileType mode;
};

#endif
