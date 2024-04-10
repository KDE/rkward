/*
scriptbackend - This file is part of RKWard (https://rkward.kde.org). Created: Sun Aug 15 2004
SPDX-FileCopyrightText: 2004-2011 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "scriptbackend.h"

#include <KLocalizedString>

#include "../plugin/rkcomponentproperties.h"

#include "../debug.h"

ScriptBackend::ScriptBackend () : QObject() {
	busy = false;
	current_type = Ignore;
	code_property = nullptr;
}

ScriptBackend::~ScriptBackend () {
	while (!command_stack.empty()) {
		delete command_stack.front();
		command_stack.pop_front();
	}
}

void ScriptBackend::callFunction (const QString &function, int flags, int type) {
	RK_TRACE (PHP);
	RK_DEBUG (PHP, DL_DEBUG, "callFunction %s", function.toLatin1 ().data ());

	ScriptCommand *command = new ScriptCommand;
	command->command = function;
	command->flags = flags;
	command->type = type;
	command->complete = false;

	if (code_property) {
		if (type == Preprocess) {
			code_property->setPreprocess (QString ());
		} else if (type == Calculate) {
			code_property->setCalculate (QString ());
		} else if (type == Printout) {
			code_property->setPrintout (QString ());
		} else if (type == Preview) {
			code_property->setPreview (QString ());
		}
		invalidateCalls (type);
	}

	command_stack.push_back(command);
	tryNextFunction ();
}

void ScriptBackend::invalidateCalls (int type) {
	RK_TRACE (PHP);

	if (current_type == type) {
		current_type = Ignore;
	}

	auto it = command_stack.begin();
	while (it != command_stack.end()) {
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
			bool add_header = add_headings && (!_output.isEmpty ());
			if (current_type == Preprocess) {
				if (add_header) code_property->setPreprocess (i18n ("## Prepare\n") + _output);
				else code_property->setPreprocess (_output);
			} else if (current_type == Calculate) {
				if (add_header) code_property->setCalculate (i18n ("## Compute\n") + _output);
				else code_property->setCalculate (_output);
			} else if (current_type == Printout) {
				if (add_header) code_property->setPrintout (i18n ("## Print result\n") + _output);
				else code_property->setPrintout (_output);
			} else if (current_type == Preview) {
				// no heading for the preview code (not shown in the code box)
				code_property->setPreview (_output);
			} else {
				Q_EMIT commandDone(current_flags);
			}
		} else {
			Q_EMIT commandDone(current_flags);
		}
	}
	busy = false;
	tryNextFunction ();
	if (!busy) {
		Q_EMIT idle();
	}
}


