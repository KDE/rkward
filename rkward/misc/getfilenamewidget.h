/***************************************************************************
                          getfilenamewidget  -  description
                             -------------------
    begin                : Tue Aug 24 2004
    copyright            : (C) 2004, 2007, 2015 by Thomas Friedrichsmeier
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
#ifndef GETFILENAMEWIDGET_H
#define GETFILENAMEWIDGET_H

#include <qwidget.h>
#include <qstring.h>

class KUrlRequester;

/**
Simple convenience class used to get a file/directoryname from the user. Basically provides a wrapper around KUrlRequester.

@author Thomas Friedrichsmeier
*/
class GetFileNameWidget : public QWidget {
	Q_OBJECT
public:
	enum FileType { ExistingFile=0, ExistingDirectory=1, SaveFile=2 };

	GetFileNameWidget (QWidget *parent, FileType mode, bool only_local, const QString &label, const QString &caption, const QString &initial);
	~GetFileNameWidget ();
	FileType getMode () const { return (_mode); };

/** set filename pattern filter, e.g. "*.cpp *.cc *.C|C++ Source Files\n*.h *.H|Header files" */
	void setFilter (const QString &filter);
/** set the filename/location from outside */
	void setLocation (const QString &new_location);

	void setBackgroundColor (const QColor & color);

/** retrieves the current location */
	QString getLocation ();
public slots:
	void locationEditChanged (const QString &);
	void updateLastUsedUrl (const QUrl& url);
signals:
	void locationChanged ();
#ifdef Q_OS_WIN
private slots:
	void hackOverrideDirDialog ();
#endif
private:
	FileType _mode;
	QString storage_key;
	KUrlRequester *edit;
};

#endif
