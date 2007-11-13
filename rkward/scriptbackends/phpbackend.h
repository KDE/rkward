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
#include <QProcess>
#include <qstringlist.h>

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
	void gotOutput ();
	void processError (QProcess::ProcessError error);
	void processDead (int exitCode, QProcess::ExitStatus exitStatus);
private:
	void tryNextFunction ();
	QProcess *php_process;
/// The string singalling the end of transmission to/from PHP. TODO: make static
	QString eot_string;
/// The string singalling the end of request from PHP. TODO: make static
	QString eoq_string;

	bool doing_command;
	bool startup_done;
	bool dead;
	QString output_raw_buffer;

	QString _output;
	QString filename;
};

#endif
