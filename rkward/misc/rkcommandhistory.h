/***************************************************************************
                          rkcommandhistory  -  description
                             -------------------
    begin                : Thu Sep 27
    copyright            : (C) 2012 by Thomas Friedrichsmeier
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