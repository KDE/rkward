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
class PHPBackend : public QObject {
	Q_OBJECT
public:
    PHPBackend();

    ~PHPBackend();

	bool initTemplate (const QString &filename, RKPlugin *responsible);
	void closeTemplate ();
/** call a PHP-function on the current template. If final is given, the output is to be considered final - i.e.
it is sent to a different location (TODO: whereever that will be) */
	void callFunction (const QString &function, bool final = false);
/** returns whether the backend is currently busy */
	bool isBusy () { return (!idle); };
public slots:
	void gotOutput (KProcess *proc, char* buf, int len);
	//void gotError (KProcess *proc, char* buf, int len);
	void doneWriting (KProcess *proc);
private:
friend class RKPlugin;
	void tryNextFunction ();
	void writeData (const QString &data);
	void tryWriteData ();
	void gotRCallResult (const QString &res);
	KProcess *php_process;
	RKPlugin *_responsible;
/// The string singalling the end of transmission to PHP. TODO: make static
	QString eot_string;
/** actually only one piece of data gets requested at a time. However, sometimes it takes a while until we realize (via doneWriting ()) that
	the last piece was transmitted ok (we have to keep a write buffer until then). So it may happen, that a new piece of information was requested,
	before we have completed the process for the previous one. Hence we use a FIFO stack just like for the commands (though it's handled slightly
	differently). */
	QStringList data_stack;
/// write buffer for the current command
	QString current_command;
	bool idle;
	bool busy_writing;
	bool doing_command;
	bool doing_final;
	
	struct PHPCommand {
	/// the temporary file where the command is stored
		KTempFile *file;
	/// whether the output is to be considered "final"
		bool is_final;
	/// whether command has finished
		bool complete;
	};
	QValueList <PHPCommand *> command_stack;
};

#endif
