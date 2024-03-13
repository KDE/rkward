/*
getfilenamewidget - This file is part of the RKWard project. Created: Tue Aug 24 2004
SPDX-FileCopyrightText: 2004-2015 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
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

	void setStyleSheet (const QString &style);

/** retrieves the current location */
	QString getLocation ();
public Q_SLOTS:
	void locationEditChanged (const QString &);
	void updateLastUsedUrl (const QUrl& url);
Q_SIGNALS:
	void locationChanged ();
#ifdef Q_OS_WIN
private Q_SLOTS:
	void hackOverrideDirDialog ();
#endif
private:
	FileType _mode;
	QString storage_key;
	KUrlRequester *edit;
};

#endif
