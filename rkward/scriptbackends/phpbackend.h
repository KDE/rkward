/***************************************************************************
                          phpbackend  -  description
                             -------------------
    begin                : Mon Jul 26 2004
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
#ifndef PHPBACKEND_H
#define PHPBACKEND_H

#include "scriptbackend.h"

#include <qstring.h>
#include <qobject.h>
#include <kprocess.h>
#include <qstringlist.h>
#include <qvaluelist.h>
#include <ktempfile.h>

class RKPlugin;

/**
This class takes care of interfacing with PHP

@author Thomas Friedrichsmeier
*/
class PHPBackend : public ScriptBackend {
	Q_OBJECT
public:
    PHPBackend();

    ~PHPBackend();

	bool initialize (const QString &filename);
	void destroy ();
	
	void preprocess (int flags) { callFunction ("preprocess ();", flags); };
	void calculate (int flags) { callFunction ("calculate ();", flags); };
	void printout (int flags) { callFunction ("printout ();", flags); };
	void cleanup (int flags) { callFunction ("cleanup ();", flags); };
	void writeData (const QString &data);
public slots:
	void gotOutput (KProcess *proc, char* buf, int len);
	//void gotError (KProcess *proc, char* buf, int len);
	void doneWriting (KProcess *proc);
private:
	void tryNextFunction ();
	void tryWriteData ();
	KProcess *php_process;
/// The string singalling the end of transmission to PHP. TODO: make static
	QString eot_string;
/** actually only one piece of data gets requested at a time. However, sometimes it takes a while until we realize (via doneWriting ()) that
	the last piece was transmitted ok (we have to keep a write buffer until then). So it may happen, that a new piece of information was requested,
	before we have completed the process for the previous one. Hence we use a FIFO stack just like for the commands (though it's handled slightly
	differently). */
	QStringList data_stack;
/// write buffer for the current command
	QString current_command;
	bool busy_writing;
	bool doing_command;
	bool startup_done;
	
	struct PHPCommand {
	/// the temporary file where the command is stored
		KTempFile *file;
	/// flags attached to this command by the parent
		int flags;
	/// whether command has finished
		bool complete;
	};
	QValueList <PHPCommand *> command_stack;

	int current_flags;
	
/** call a PHP-function on the current template. */
	void callFunction (const QString &function, int flags);
};

#endif
