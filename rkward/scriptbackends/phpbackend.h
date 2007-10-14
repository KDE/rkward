/***************************************************************************
                          phpbackend  -  description
                             -------------------
    begin                : Mon Jul 26 2004
    copyright            : (C) 2004, 2007 by Thomas Friedrichsmeier
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
#include <k3process.h>
#include <qstringlist.h>
#include <q3valuelist.h>

class RKPlugin;

/**
This class takes care of interfacing with PHP

@author Thomas Friedrichsmeier
*/
class PHPBackend : public ScriptBackend {
	Q_OBJECT
public:
	PHPBackend (const QString &filename);

	~PHPBackend ();

	bool initialize (RKComponentPropertyCode *code_property=0, bool add_headings=true);
	void destroy ();
	
	void preprocess (int flags) { callFunction ("preprocess ();", flags, Preprocess); };
	void calculate (int flags) { callFunction ("calculate ();", flags, Calculate); };
	void printout (int flags) { callFunction ("printout ();", flags, Printout); };
	void preview (int flags) { callFunction ("getPreview ();", flags, Preview); };
	void writeData (const QString &data);
public slots:
	void gotOutput (K3Process *proc, char* buf, int len);
	//void gotError (K3Process *proc, char* buf, int len);
	void processDied (K3Process *proc);
	void doneWriting (K3Process *proc);
private:
	void tryNextFunction ();
	void tryWriteData ();
	K3Process *php_process;
/// The string singalling the end of transmission to/from PHP. TODO: make static
	QString eot_string;
/// The string singalling the end of request from PHP. TODO: make static
	QString eoq_string;
/** actually only one piece of data gets requested at a time. However, sometimes it takes a while until we realize (via doneWriting ()) that the last piece was transmitted ok (we have to keep a write buffer until then). So it may happen, that a new piece of information was requested, before we have completed the process for the previous one. Hence we use a FIFO stack just like for the commands (though it's handled slightly differently). */
	QStringList data_stack;
/// write buffer for the current command
	QString current_command;
	bool busy_writing;
	bool doing_command;
	bool startup_done;
	QString output_raw_buffer;

	QString _output;
	QString filename;
};

#endif
