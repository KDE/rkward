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
#include "phpbackend.h"

#include "stdio.h"

#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>

#include <kmessagebox.h>
#include <klocale.h>

#include "../misc/rkcommonfunctions.h"
#include "../settings/rksettingsmodulephp.h"
#include "../plugin/rkcomponentproperties.h"
#include "../debug.h"

PHPBackend::PHPBackend() {
	RK_TRACE (PHP);

	php_process = 0;
	eot_string="#RKEND#\n";
	eoq_string="#RKQEND#\n";
	busy_writing = false;
	busy = false;
}


PHPBackend::~PHPBackend() {
	RK_TRACE (PHP);
	destroy ();
}

bool PHPBackend::initialize (const QString &filename, RKComponentPropertyCode *code_property, bool add_headings) {
	RK_TRACE (PHP);

	if (php_process && php_process->isRunning ()) {
		RK_DO (qDebug ("another template is already openend in this backend"), PHP, DL_ERROR);
		return false;
	}

	QDir files_path (RKCommonFunctions::getRKWardDataDir () + "phpfiles/");
	QString common_php = files_path.filePath ("common.php", false);
	QString php_ini = files_path.filePath ("php_ini", false);
	if (!QFileInfo (common_php).isReadable ()) {
		KMessageBox::error (0, i18n ("The support file \"%1\" could not be found or is not readable. Please check your installation.").arg (common_php), i18n ("PHP-Error"));
		emit (haveError ());
		return false;
	}

	php_process = new KProcess ();
	*php_process << RKSettingsModulePHP::phpBin();
	*php_process << "-c" << php_ini;	// set correct options
	*php_process << common_php;
	
	// we have to be connect at all times! Otherwise the connection will be gone for good.
	//connect (php_process, SIGNAL (receivedStderr (KProcess *, char*, int)), this, SLOT (gotError (KProcess *, char*, int)));
	connect (php_process, SIGNAL (wroteStdin (KProcess *)), this, SLOT (doneWriting (KProcess* )));
	connect (php_process, SIGNAL (receivedStdout (KProcess *, char*, int)), this, SLOT (gotOutput (KProcess *, char*, int)));
	connect (php_process, SIGNAL (processExited (KProcess *)), this, SLOT (processDied (KProcess*)));
	
	if (!php_process->start (KProcess::NotifyOnExit, KProcess::All)) {
		KMessageBox::error (0, i18n ("The PHP backend could not be started. Check whether you have correctly configured the location of the PHP-binary (Settings->Configure Settings->PHP backend)"), i18n ("PHP-Error"));
		emit (haveError ());
		return false;
	}

	busy_writing = doing_command = startup_done = false;
	busy = true;

	// start the real template
	callFunction ("include (\"" + filename + "\");", 0, Ignore);

	PHPBackend::code_property = code_property;
	PHPBackend::add_headings = add_headings;
	return true;
}

void PHPBackend::destroy () {
	RK_TRACE (PHP);

	if (php_process) {
		php_process->detach ();
		php_process->deleteLater ();
		php_process = 0;
	}
	
	busy_writing = false;
	busy = false;
	
	while (command_stack.count ()) {
		delete command_stack.first ();
		command_stack.pop_front ();
	}
	
	data_stack.clear ();
}

void PHPBackend::callFunction (const QString &function, int flags, int type) {
	RK_TRACE (PHP);
	RK_DO (qDebug ("callFunction %s", function.latin1 ()), PHP, DL_DEBUG);

	PHPCommand *command = new PHPCommand;
	command->command = function;
	command->flags = flags;
	command->type = type;
	command->complete = false;

	if (code_property) {
		if (type == Preprocess) {
			code_property->setPreprocess (QString::null);
		} else if (type == Calculate) {
			code_property->setCalculate (QString::null);
		} else if (type == Printout) {
			code_property->setPrintout (QString::null);
		} else if (type == Cleanup) {
			code_property->setCleanup (QString::null);
		} else if (type == Preview) {
			code_property->setPreview (QString::null);
		}
		invalidateCalls (type);
	}

	command_stack.append (command);
	tryNextFunction ();
}

void PHPBackend::invalidateCalls (int type) {
	RK_TRACE (PHP);

	if (current_type == type) {
		current_type = Ignore;
	}

	QValueList<PHPCommand *>::iterator it = command_stack.begin ();
	while (it != command_stack.end ()) {
		if ((*it)->type == type) {
			delete (*it);
			it = command_stack.erase (it);		// it now points to next item
		} else {
			++it;
		}
	}
}


void PHPBackend::tryNextFunction () {
	RK_TRACE (PHP);

	if ((!busy_writing) && php_process && php_process->isRunning () && (!busy) && (!command_stack.isEmpty ())) {
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
		current_type = command_stack.first ()->type;
	}
}

void PHPBackend::writeData (const QString &data) {
	RK_TRACE (PHP);
	data_stack.append (data  + eot_string);
	tryWriteData ();
}

void PHPBackend::tryWriteData () {
	RK_TRACE (PHP);

	if ((!busy_writing) && php_process && php_process->isRunning () && busy && (!data_stack.isEmpty ())) {
		RK_DO (qDebug ("submitting data: %s", data_stack.first ().latin1 ()), PHP, DL_DEBUG);
		php_process->writeStdin (data_stack.first ().latin1 (), data_stack.first ().length ());
		busy_writing = true;
		doing_command = false;
	}
}

void PHPBackend::doneWriting (KProcess *) {
	RK_TRACE (PHP);

	busy_writing = false;
	if (!doing_command) data_stack.pop_front ();
	tryWriteData ();
	tryNextFunction ();
}

void PHPBackend::gotOutput (KProcess *, char* buf, int) {
	RK_TRACE (PHP);

	RK_DO (qDebug ("PHP transmission:\n%s", buf), PHP, DL_DEBUG);

	output_raw_buffer += buf;
	QString request;
	QString data;
	int i, j;
	bool have_data = true;
	bool have_request = false;

	// is there a request in the output stream?
	if ((i = output_raw_buffer.find (eot_string)) >= 0) {
		if ((j = output_raw_buffer.find (eoq_string, i)) >= 0) {
			have_request = true;
			// is there also pending data?
			if (i) {
				data = output_raw_buffer.left (i);
			} else {
				have_data = false;
			}
			int start = i + eot_string.length ();
			request = output_raw_buffer.mid (start, j - start);
			output_raw_buffer = QString::null;
		}
	} else {
		data = output_raw_buffer;
	}
	
	// pending data is always first in a stream, so process it first, too
	if (have_data) {
		if (!startup_done) {
				php_process->detach ();
				KMessageBox::error (0, i18n ("There has been an error\n(\"%1\")\nwhile starting up the PHP backend. Most likely this is due to either a bug in RKWard or an invalid setting for the location of the PHP support files. Check the settings (Settings->Configure Settings->PHP backend) and try again.").arg (output_raw_buffer.stripWhiteSpace ()), i18n ("PHP-Error"));
				emit (haveError ());
				destroy ();
				return;
		}
	}

	if (!have_request) {
		// output is not finished, yet.
		// return and wait for more data to come in
		RK_DO (qDebug ("PHP transmission not complete, yet"), PHP, DL_DEBUG);
		return;
	} else {
		_output.append (data);
		RK_DO (qDebug ("request: %s\ndata: %s", request.latin1 (), data.latin1 ()), PHP, DL_DEBUG);

		if (request == "requesting code") {
			startup_done = true;
			busy = false;
			RK_DO (qDebug ("got type: %d, stack %d", current_type, command_stack.count ()), PHP, DL_DEBUG);
			if (current_type != Ignore) {
				if (code_property) {
					if (_output.isNull ()) _output = "";			// must not be null for the code property!
					if (current_type == Preprocess) {
						if (add_headings) code_property->setPreprocess (i18n ("## Prepare\n") + retrieveOutput ());
						else code_property->setPreprocess (retrieveOutput ());
						resetOutput ();
					} else if (current_type == Calculate) {
						if (add_headings) code_property->setCalculate (i18n ("## Compute\n") + retrieveOutput ());
						else code_property->setCalculate (retrieveOutput ());
						resetOutput ();
					} else if (current_type == Printout) {
						if (add_headings) code_property->setPrintout (i18n ("## Print result\n") + retrieveOutput ());
						else code_property->setPrintout (retrieveOutput ());
						resetOutput ();
					} else if (current_type == Cleanup) {
						if (add_headings) code_property->setCleanup (i18n ("## Clean up\n") + retrieveOutput ());
						else code_property->setCleanup (retrieveOutput ());
						resetOutput ();
					} else if (current_type == Preview) {
						// no heading for the preview code (not shown in the code box)
						code_property->setPreview (retrieveOutput ());
						resetOutput ();
					} else {
						emit (commandDone (current_flags));
					}
				} else {
					emit (commandDone (current_flags));
				}
			}
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
		} else if (request.startsWith ("PHP-Error")) {
			QString error = request.remove ("PHP-Error");
			php_process->detach ();
			KMessageBox::error (0, i18n ("The PHP-backend has reported an error\n(\"%1\")\nand has been shut down. This is most likely due to a bug in the plugin. But of course you may want to try to close and restart the plugin to see whether it works with different settings.").arg (error.stripWhiteSpace ()), i18n ("PHP-Error"));
			emit (haveError ());
			destroy ();
			return;
		} else {
			RK_DO (qDebug ("unrecognized request from PHP backend: \"%s\"", request.latin1()), PHP, DL_ERROR);
		}
	}
}

void PHPBackend::processDied (KProcess *) {
	RK_TRACE (PHP);

	php_process->detach ();
	KMessageBox::error (0, i18n ("The PHP-backend has died unexpectedly. The current output buffer is shown below:\n%1").arg (output_raw_buffer), i18n ("PHP Process exited"));
	emit (haveError ());
	destroy ();
}


#include "phpbackend.moc"
