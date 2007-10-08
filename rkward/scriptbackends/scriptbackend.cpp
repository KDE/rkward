/***************************************************************************
                          scriptbackend  -  description
                             -------------------
    begin                : Sun Aug 15 2004
    copyright            : (C) 2004, 2006 by Thomas Friedrichsmeier
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
#include "scriptbackend.h"

#include <klocale.h>

#include "../plugin/rkcomponentproperties.h"

#include "../debug.h"
//Added by qt3to4:
#include <Q3ValueList>

ScriptBackend::ScriptBackend () : QObject() {
	busy = false;
}

ScriptBackend::~ScriptBackend () {
}

void ScriptBackend::callFunction (const QString &function, int flags, int type) {
	RK_TRACE (PHP);
	RK_DO (qDebug ("callFunction %s", function.toLatin1 ()), PHP, DL_DEBUG);

	ScriptCommand *command = new ScriptCommand;
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
		} else if (type == Preview) {
			code_property->setPreview (QString::null);
		}
		invalidateCalls (type);
	}

	command_stack.append (command);
	tryNextFunction ();
}

void ScriptBackend::invalidateCalls (int type) {
	RK_TRACE (PHP);

	if (current_type == type) {
		current_type = Ignore;
	}

	Q3ValueList<ScriptCommand *>::iterator it = command_stack.begin ();
	while (it != command_stack.end ()) {
		if ((*it)->type == type) {
			delete (*it);
			it = command_stack.erase (it);		// it now points to next item
		} else {
			++it;
		}
	}
}

void ScriptBackend::commandFinished (const QString &output) {
	RK_TRACE (PHP);

	QString _output = output;

	if (current_type != Ignore) {
		if (code_property) {
			if (_output.isNull ()) _output = "";			// must not be null for the code property!
			if (current_type == Preprocess) {
				if (add_headings) code_property->setPreprocess (i18n ("## Prepare\n") + _output);
				else code_property->setPreprocess (_output);
			} else if (current_type == Calculate) {
				if (add_headings) code_property->setCalculate (i18n ("## Compute\n") + _output);
				else code_property->setCalculate (_output);
			} else if (current_type == Printout) {
				if (add_headings) code_property->setPrintout (i18n ("## Print result\n") + _output);
				else code_property->setPrintout (_output);
			} else if (current_type == Preview) {
				// no heading for the preview code (not shown in the code box)
				code_property->setPreview (_output);
			} else {
				emit (commandDone (current_flags));
			}
		} else {
			emit (commandDone (current_flags));
		}
	}
	busy = false;
	tryNextFunction ();
	if (!busy) {
		emit (idle ());
	}
}


#include "scriptbackend.moc"
