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
#include "phpbackend.h"

#include "stdio.h"

#include <qfile.h>

#include "rkplugin.h"

PHPBackend::PHPBackend() {
	qDebug ("TODO: make location of PHP-binary configurable");
	php_process = 0;
	eot_string="#RKEND#\n";
	idle = busy_writing = false;
	doing_final = false;
}


PHPBackend::~PHPBackend() {
	closeTemplate ();
}

bool PHPBackend::initTemplate (const QString &filename, RKPlugin *responsible) {
	if (php_process && php_process->isRunning ()) {
		qDebug ("another template is already openend in this backend");
		return false;
	}

	_responsible = responsible;
	
	php_process = new KProcess (this);
	*php_process << "php";
	*php_process << "common.php";
	
	// we have to be connect at all times! Otherwise the connection will be gone for good.
	//connect (php_process, SIGNAL (receivedStderr (KProcess *, char*, int)), this, SLOT (gotError (KProcess *, char*, int)));
	connect (php_process, SIGNAL (wroteStdin (KProcess *)), this, SLOT (doneWriting (KProcess* )));
	connect (php_process, SIGNAL (receivedStdout (KProcess *, char*, int)), this, SLOT (gotOutput (KProcess *, char*, int)));
	
	if (!php_process->start (KProcess::NotifyOnExit, KProcess::All)) {
		qDebug ("could not launch php");
		return false;
	}

	idle = busy_writing = doing_command = false;

	// start the real template
	callFunction ("include (\"" + filename + "\");");
		
	return true;
}

void PHPBackend::closeTemplate () {
	if (php_process) php_process->kill ();
	delete php_process;
	php_process = 0;
	
	idle = busy_writing = false;
	
	while (command_stack.count ()) {
		command_stack.first ()->file->unlink ();
		delete command_stack.first ()->file;
		delete command_stack.first ();
		command_stack.pop_front ();
	}
	
	data_stack.clear ();
}

void PHPBackend::callFunction (const QString &function, bool final) {
	qDebug ("callFunction %s", function.latin1 ());
	KTempFile *file = new KTempFile ();
	*(file->textStream ()) << "<? " << function << " ?>\n";
	file->close ();
	PHPCommand *command = new PHPCommand;
	command->file = file;
	command->is_final = final;
	command->complete = false;
	command_stack.append (command);
	tryNextFunction ();
}

void PHPBackend::tryNextFunction () {
	if ((!busy_writing) && php_process && php_process->isRunning () && idle && command_stack.count ()) {
	/// clean up previous command if applicable
		if (command_stack.first ()->complete) {
			command_stack.first ()->file->unlink ();
			delete command_stack.first ()->file;
			delete command_stack.first ();
			command_stack.pop_front ();
			
			if (!command_stack.count ()) return;
		}
		
		qDebug ("submitting PHP code: " + command_stack.first ()->file->name ());
		current_command = command_stack.first ()->file->name () + eot_string;
		php_process->writeStdin (current_command.latin1 (), current_command.length ());
		busy_writing = doing_command = true;
		idle = false;
		doing_final = command_stack.first ()->is_final;
		command_stack.first ()->complete = true;
	}
}

void PHPBackend::writeData (const QString &data) {
	data_stack.append (data);
	tryWriteData ();
}

void PHPBackend::tryWriteData () {
	if ((!busy_writing) && php_process && php_process->isRunning () && (!idle) && (data_stack.count ())) {
		qDebug ("submitting data: " + data_stack.first ());
		php_process->writeStdin (data_stack.first ().latin1 (), data_stack.first ().length ());
		busy_writing = true;
		doing_command = false;
	}
}

void PHPBackend::doneWriting (KProcess *proc) {
	busy_writing = false;
	if (!doing_command) data_stack.pop_front ();
	tryWriteData ();
}

void PHPBackend::gotOutput (KProcess *proc, char* buf, int len) {
	QString output = buf;
	QString request;
	QString data;
	int i;
	bool have_data = true;;
	bool have_request = false;
	// is there a request in the output stream?
	if ((i = output.find (eot_string)) >= 0) {
		have_request = true;
		// is there also pending data?
		if (i) {
			data = output.left (i);
		} else {
			have_data = false;
		}
		request = output.mid (i + eot_string.length (), len);
	} else {
		data = output;
	}
	qDebug ("request: %s\ndata: %s", request.latin1 (), data.latin1 ());
	
	// pending data is always first in a stream, so process it first, too
	if (have_data) {
		if (!doing_final) {
			_responsible->updateCode (data);
		} else {
			qDebug ("got final output!");

			QFile file ("rk_out.html");
			if (!file.open (IO_WriteOnly| IO_Append)) {
				qDebug ("failed to open outfile");
				file.close ();
				return;
			}
			QTextStream (&file) << data;
			file.close ();

			_responsible->newOutput ();
		}
	}
	
	if (have_request) {
		if (request == "requesting code") {
			idle = true;
			tryNextFunction ();
			if (idle) {
				if (_responsible->backendIdle()) return;	// self-destruct imminent. quit
			} 
		} else if (request.startsWith ("requesting data:")) {
			QString requested_object = request.remove ("requesting data:");
			qDebug ("requested data: \"" + requested_object + "\"");
			QString res = _responsible->getVar (requested_object);
			idle = false;
			writeData (res + eot_string);
		} else if (request.startsWith ("requesting rcall:")) {
			QString requested_call = request.remove ("requesting rcall:");
			qDebug ("requested rcall: \"" + requested_call + "\"");
			_responsible->doRCall (requested_call);
			idle = false;
		}
		return;
	}
}

void PHPBackend::gotRCallResult (const QString &res) {
	writeData (res + eot_string);	
}