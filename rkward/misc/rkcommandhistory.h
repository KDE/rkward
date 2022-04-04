/*
rkcommandhistory - This file is part of the RKWard project. Created: Thu Sep 27
SPDX-FileCopyrightText: 2012 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKCOMMANDHISTORY_H
#define RKCOMMANDHISTORY_H

#include <QStringList>

/** Simple class to manage a history of command strings (esp. for the RKConsole) */
class RKCommandHistory {
public:
	RKCommandHistory (bool allow_empty, bool allow_dupes);
	~RKCommandHistory ();

	bool up (bool context_sensitive = false, const QString& current_context = QString ());
	bool down (bool context_sensitive = false, const QString& current_context = QString ());
	QString current () const { return history.value (current_position, editing_line); };
	void append (const QString& new_line);
	void goToEnd () { editing_line.clear (); current_position = history.size (); }

	QStringList getHistory () const { return history; }
	void setHistory (const QStringList& _history, bool append=false);
private:
	void trim ();
	bool allow_empty;
	bool allow_dupes;
	int current_position;
	QString _current_context;
	QString editing_line;
	QStringList history;
};

#endif
