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

#include <kmessagebox.h>
#include <klocale.h>

#include "../settings/rksettingsmodulephp.h"
#include "../debug.h"

PHPBackend::PHPBackend() {
	RK_TRACE (PHP);

	php_process = 0;
	eot_string="#RKEND#\n";
	busy_writing = false;
	busy = false;
}


PHPBackend::~PHPBackend() {
	RK_TRACE (PHP);
	destroy ();
}

bool PHPBackend::initialize (const QString &filename) {
	RK_TRACE (PHP);

	if (php_process && php_process->isRunning ()) {
		RK_DO (qDebug ("another template is already openend in this backend"), PHP, DL_ERROR);
		return false;
	}

	php_process = new KProcess ();
	*php_process << RKSettingsModulePHP::phpBin();
	*php_process << (RKSettingsModulePHP::filesPath() + "/common.php");
	
	// we have to be connect at all times! Otherwise the connection will be gone for good.
	//connect (php_process, SIGNAL (receivedStderr (KProcess *, char*, int)), this, SLOT (gotError (KProcess *, char*, int)));
	connect (php_process, SIGNAL (wroteStdin (KProcess *)), this, SLOT (doneWriting (KProcess* )));
	connect (php_process, SIGNAL (receivedStdout (KProcess *, char*, int)), this, SLOT (gotOutput (KProcess *, char*, int)));
	
	if (!php_process->start (KProcess::NotifyOnExit, KProcess::All)) {
		KMessageBox::error (0, i18n ("The PHP backend could not be started. Check whether you have correctly configured the location of the PHP-binary (Settings->Configure Settings->PHP backend)"), i18n ("PHP-Error"));
		emit (haveError ());
		return false;
	}

	busy_writing = doing_command = startup_done = false;
	busy = true;
		
	// start the real template
	callFunction ("include (\"" + filename + "\");", 0);
		
	return true;
}

void PHPBackend::destroy () {
	RK_TRACE (PHP);

	if (php_process) php_process->kill ();
	delete php_process;
	php_process = 0;
	
	busy_writing = false;
	busy = false;
	
	while (command_stack.count ()) {
		delete command_stack.first ();
		command_stack.pop_front ();
	}
	
	data_stack.clear ();
}

void PHPBackend::callFunction (const QString &function, int flags) {
	RK_TRACE (PHP);
	RK_DO (qDebug ("callFunction %s", function.latin1 ()), PHP, DL_DEBUG);

	PHPCommand *command = new PHPCommand;
	command->command = function;
	command->flags = flags;
	command->complete = false;
	command_stack.append (command);
	tryNextFunction ();
}

void PHPBackend::tryNextFunction () {
	RK_TRACE (PHP);

	if ((!busy_writing) && php_process && php_process->isRunning () && (!busy) && command_stack.count ()) {
	/// clean up previous command if applicable
		if (command_stack.first ()->complete) {
			delete command_stack.first ();
			command_stack.pop_front ();
			
			if (!command_stack.count ()) return;
		}
		
		RK_DO (qDebug ("submitting PHP code: %s", command_stack.first ()->command.latin1 ()), PHP, DL_DEBUG);
		current_command = command_stack.first ()->command + eot_string;
		php_process->writeStdin (current_command.latin1 (), current_command.length ());
		busy_writing = doing_command = busy = true;
		command_stack.first ()->complete = true;
		current_flags = command_stack.first ()->flags;
	}
}

void PHPBackend::writeData (const QString &data) {
	RK_TRACE (PHP);
	data_stack.append (data  + eot_string);
	tryWriteData ();
}

void PHPBackend::tryWriteData () {
	RK_TRACE (PHP);

	if ((!busy_writing) && php_process && php_process->isRunning () && busy && (data_stack.count ())) {
		RK_DO (qDebug ("submitting data: %s", data_stack.first ().latin1 ()), PHP, DL_DEBUG);
		php_process->writeStdin (data_stack.first ().latin1 (), data_stack.first ().length ());
		busy_writing = true;
		doing_command = false;
	}
}

void PHPBackend::doneWriting (KProcess *proc) {
	RK_TRACE (PHP);

	busy_writing = false;
	if (!doing_command) data_stack.pop_front ();
	tryWriteData ();
}

void PHPBackend::gotOutput (KProcess *proc, char* buf, int len) {
	RK_TRACE (PHP);

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
	RK_DO (qDebug ("request: %s\ndata: %s", request.latin1 (), data.latin1 ()), PHP, DL_DEBUG);
	
	// pending data is always first in a stream, so process it first, too
	if (have_data) {
		if (!startup_done) {
				destroy ();
				KMessageBox::error (0, i18n ("There has been an error\n(\"%1\")\nwhile starting up the PHP backend. Most likely this is due to either a bug in RKward or an invalid setting for the location of the PHP support files. Check the settings (Settings->Configure Settings->PHP backend) and try again.").arg (data.stripWhiteSpace ()), i18n ("PHP-Error"));
				emit (haveError ());
				return;
		}
		
		_output.append (data);
	}
	
	if (have_request) {
		if (request == "requesting code") {
			startup_done = true;
			busy = false;
			emit (commandDone (current_flags));
			tryNextFunction ();
			if (!busy) {
				emit (idle ());
				return;
			} 
		} else if (request.startsWith ("requesting data:")) {
			QString requested_object = request.remove ("requesting data:");
			RK_DO (qDebug ("requested data: \"%s\"", requested_object.latin1 ()), PHP, DL_DEBUG);
			emit (requestValue (requested_object));
			busy = true;
//			writeData (res + eot_string);
		} else if (request.startsWith ("requesting rcall:")) {
			QString requested_call = request.remove ("requesting rcall:");
			RK_DO (qDebug ("requested rcall: \"%s\"", requested_call.latin1 ()), PHP, DL_DEBUG);
			emit (requestRCall (requested_call));
			busy = true;
//			_responsible->doRCall (requested_call);
		} else if (request.startsWith ("requesting rvector:")) {
			QString requested_call = request.remove ("requesting rvector:");
			RK_DO (qDebug ("requested rvector: \"%s\"", requested_call.latin1 ()), PHP, DL_DEBUG);
			emit (requestRVector (requested_call));
			busy = true;
//			_responsible->getRVector (requested_call);
		} else if (request.startsWith ("PHP-Error")) {
				QString error = request.remove ("PHP-Error");
				destroy ();
				KMessageBox::error (0, i18n ("The PHP-backend has reported an error\n(\"%1\")\nand has been shut down. This is most likely due to a bug in the plugin. But of course you may want to try to close and restart the plugin to see whether it works with different settings.").arg (error.stripWhiteSpace ()), i18n ("PHP-Error"));
				emit (haveError ());
				return;
		}
		return;
	}
}

#include "phpbackend.moc"
