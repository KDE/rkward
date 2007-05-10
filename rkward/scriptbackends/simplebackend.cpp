/***************************************************************************
                          simplebackend  -  description
                             -------------------
    begin                : Thu May 10 2007
    copyright            : (C) 2007 by Thomas Friedrichsmeier
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

#include "simplebackend.h"

#include "../debug.h"

SimpleBackend::SimpleBackend () : ScriptBackend () {
	RK_TRACE (PHP);
}

SimpleBackend::~SimpleBackend () {
	RK_TRACE (PHP);
}

bool SimpleBackend::initialize (RKComponentPropertyCode *code_property, bool add_headings) {
	RK_TRACE (PHP);

	SimpleBackend::code_property = code_property;
	SimpleBackend::add_headings = add_headings;
	template_pos = 0;

	return true;
}

void SimpleBackend::destroy () {
	RK_TRACE (PHP);

	deleteLater ();
}

void SimpleBackend::preprocess (int flags) {
	RK_TRACE (PHP);

	callFunction (QString::null, flags, Preprocess);
}

void SimpleBackend::calculate (int flags) {
	RK_TRACE (PHP);

	callFunction (QString::null, flags, Calculate);
}

void SimpleBackend::printout (int flags) {
	RK_TRACE (PHP);

	callFunction (QString::null, flags, Printout);
}

void SimpleBackend::preview (int flags) {
	RK_TRACE (PHP);

	callFunction (QString::null, flags, Preview);
}

void SimpleBackend::writeData (const QString &data) {
	RK_TRACE (PHP);

	current_values.append (data);
	processCall ();
}

void SimpleBackend::tryNextFunction () {
	RK_TRACE (PHP);

	if ((!busy) && (!command_stack.isEmpty ())) {
		// clean up previous command if applicable
		if (command_stack.first ()->complete) {
			delete command_stack.first ();
			command_stack.pop_front ();

			if (!command_stack.count ()) return;
		}

		busy = true;
		command_stack.first ()->complete = true;
		current_flags = command_stack.first ()->flags;
		current_type = command_stack.first ()->type;

		current_values.clear ();
		template_pos = 0;
		if (current_type == Preprocess) current_template = preprocess_template;
		else if (current_type == Printout) current_template = printout_template;
		else if (current_type == Calculate) current_template = calculate_template;
		else if (current_type == Preview) current_template = preview_template;
		template_sep = current_template.find ("!!!");

		if (template_sep < 0) {
			commandFinished ("");
			return;
		}

		processCall ();
	}
}

void SimpleBackend::processCall () {
	RK_TRACE (PHP);

	int next_token = current_template.find ("$$$", template_pos);
	if (next_token < 0) next_token = template_sep;
	if (next_token > template_sep) next_token = template_sep;

	if (next_token < template_sep) {
		int token_end = current_template.find ("$$$", next_token + 3);
		RK_ASSERT (token_end >= 0);
		QString token = current_template.mid (next_token + 3, token_end - (next_token + 3));
		template_pos = token_end + 3;
		emit (requestValue (token));
		return;
	}

	// all values are fetched. Now generate the return string
	finishCall (current_template.mid (template_sep + 3));
}

void SimpleBackend::finishCall (const QString &conditions) {
	RK_TRACE (PHP);

	QString conds = conditions;
	int repl = current_values.count();
	for (int i = repl; i > 0; --i) {
		QString placeholder = "%" + QString::number (i);
		QString replacement = current_values[i-1];
		conds.replace (placeholder, replacement);
	}

	QString output;
	int pos = 3;
	int max = conds.length ();
	do {
		int cond_end = conds.find ("!?!", pos);
		if (cond_end < 0) cond_end = max;
		QString condition = conds.mid (pos, cond_end - pos);

		int if_end = condition.find ("!:!");
		RK_ASSERT (if_end >= 0);
		QString if_part = condition.left (if_end);

		int if_mid = if_part.find ("!=!");
		RK_ASSERT (if_mid >= 0);
		QString if_compare = if_part.left (if_mid);

		QString if_against = if_part.mid (if_mid + 3);
		if ((if_compare.isEmpty() && if_against.isEmpty ()) || (if_compare == if_against)) {
			output = condition.mid (if_end + 3);
			break;
		}

		pos = cond_end + 3;
	} while (pos < max);

	// reached end of template
	commandFinished (output);
}
