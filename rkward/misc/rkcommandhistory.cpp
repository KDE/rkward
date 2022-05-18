/*
rkcommandhistory - This file is part of RKWard (https://rkward.kde.org). Created: Thu Sep 27
SPDX-FileCopyrightText: 2012 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkcommandhistory.h"
#include "../settings/rksettingsmoduleconsole.h"

#include "../debug.h"

RKCommandHistory::RKCommandHistory (bool allow_empty, bool allow_dupes) {
	RK_TRACE (MISC);

	current_position = 0;
	RKCommandHistory::allow_empty = allow_empty;
	RKCommandHistory::allow_dupes = allow_dupes;
}

RKCommandHistory::~RKCommandHistory () {
	RK_TRACE (MISC);
}

bool RKCommandHistory::up (bool context_sensitive, const QString& current_context) {
	RK_TRACE (MISC);

	int new_pos;
	if (context_sensitive) {
		QString filter = current_context;
		if (filter == current ()) {	// user is just browsing through the list. Use previous filter string
			filter = _current_context;
		} else {	// line appears edited. Use as new filter
			_current_context = filter;
		}

		new_pos = -1;
		for (int i = qMin (current_position - 1, history.size () - 1); i >= 0; --i) {
			if (history[i].startsWith (filter)) {
				new_pos = i;
				break;
			}
		}
	} else {
		new_pos = current_position - 1;
	}

	if (new_pos >= 0) {
		if (current_position == (history.size ())) {	// on last line (and going up)? Auto append current context
			editing_line = current_context;
		}
		current_position = new_pos;
		return true;
	}
	return false;
}

bool RKCommandHistory::down (bool context_sensitive, const QString& current_context) {
	RK_TRACE (MISC);

	int new_pos;
	if (context_sensitive) {
		QString filter = current_context;
		if (filter == current ()) {	// user is just browsing through the list. Use previous filter string
			filter = _current_context;
		} else {	// line appears edited. Use as new filter
			_current_context = filter;
		}

		new_pos = history.size ();
		for (int i = current_position + 1; i < history.size (); ++i) {
			if (history[i].startsWith (filter)) {
				new_pos = i;
				break;
			}
		}
	} else {
		new_pos = current_position + 1;
	}

	if (new_pos <= history.size ()) {
		current_position = new_pos;
		return true;
	}
	return false;
}

void RKCommandHistory::trim () {
	if (RKSettingsModuleConsole::maxHistoryLength ()) {
		for (uint ui = history.size (); ui > RKSettingsModuleConsole::maxHistoryLength (); --ui) {
			history.pop_front ();
		}
	}
}


void RKCommandHistory::append (const QString& new_line) {
	RK_TRACE (MISC);

	if (allow_empty || (!new_line.isEmpty ())) {
		if (allow_dupes || (new_line != history.value (history.size () - 1))) {
			history.append (new_line);
			trim ();
		}
	}
	goToEnd ();
}

void RKCommandHistory::setHistory (const QStringList& _history, bool append) {
	RK_TRACE (MISC);

	if (append) history.append (_history);
	else history = _history;

	trim ();
	goToEnd ();
}
