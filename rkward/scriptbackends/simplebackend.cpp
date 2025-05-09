/*
simplebackend - This file is part of RKWard (https://rkward.kde.org). Created: Thu May 10 2007
SPDX-FileCopyrightText: 2007-2012 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "simplebackend.h"

#include "../debug.h"

SimpleBackend::SimpleBackend() : ScriptBackend() {
	RK_TRACE(PHP);
}

SimpleBackend::~SimpleBackend() {
	RK_TRACE(PHP);
}

bool SimpleBackend::initialize(RKComponentPropertyCode *code_property, bool add_headings) {
	RK_TRACE(PHP);

	SimpleBackend::code_property = code_property;
	SimpleBackend::add_headings = add_headings;
	template_pos = 0;

	return true;
}

void SimpleBackend::destroy() {
	RK_TRACE(PHP);

	deleteLater();
}

void SimpleBackend::preprocess(int flags) {
	RK_TRACE(PHP);

	callFunction(QString(), flags, Preprocess);
}

void SimpleBackend::calculate(int flags) {
	RK_TRACE(PHP);

	callFunction(QString(), flags, Calculate);
}

void SimpleBackend::printout(int flags) {
	RK_TRACE(PHP);

	callFunction(QString(), flags, Printout);
}

void SimpleBackend::preview(int flags) {
	RK_TRACE(PHP);

	callFunction(QString(), flags, Preview);
}

void SimpleBackend::writeData(const QVariant &data) {
	RK_TRACE(PHP);

	current_values.append(data);
	processCall();
}

void SimpleBackend::tryNextFunction() {
	RK_TRACE(PHP);

	if ((!busy) && (!command_stack.empty())) {
		// clean up previous command if applicable
		if (command_stack.front()->complete) {
			delete command_stack.front();
			command_stack.pop_front();

			if (command_stack.empty()) return;
		}

		busy = true;
		command_stack.front()->complete = true;
		current_flags = command_stack.front()->flags;
		current_type = command_stack.front()->type;

		current_values.clear();
		template_pos = 0;
		if (current_type == Preprocess) current_template = preprocess_template;
		else if (current_type == Printout) current_template = printout_template;
		else if (current_type == Calculate) current_template = calculate_template;
		else if (current_type == Preview) current_template = preview_template;
		template_sep = current_template.indexOf(QLatin1String("!!!"));

		if (template_sep < 0) {
			commandFinished(QLatin1String(""));
			return;
		}

		processCall();
	}
}

void SimpleBackend::processCall() {
	RK_TRACE(PHP);

	int next_token = current_template.indexOf(QLatin1String("$$$"), template_pos);
	if (next_token < 0) next_token = template_sep;
	if (next_token > template_sep) next_token = template_sep;

	if (next_token < template_sep) {
		int token_end = current_template.indexOf(QLatin1String("$$$"), next_token + 3);
		RK_ASSERT(token_end >= 0);
		QString token = current_template.mid(next_token + 3, token_end - (next_token + 3));
		template_pos = token_end + 3;
		Q_EMIT requestValue(token, RKStandardComponent::StringValue);
		return;
	}

	// all values are fetched. Now generate the return string
	finishCall(current_template.mid(template_sep + 3));
}

void SimpleBackend::finishCall(const QString &conditions) {
	RK_TRACE(PHP);

	QString conds = conditions;
	int repl = current_values.count();
	for (int i = repl; i > 0; --i) {
		QString placeholder = u'%' + QString::number(i);
		QString replacement = current_values[i - 1].toString();
		conds.replace(placeholder, replacement);
	}

	QString output;
	int pos = 3;
	int max = conds.length();
	do {
		int cond_end = conds.indexOf(QLatin1String("!?!"), pos);
		if (cond_end < 0) cond_end = max;
		QString condition = conds.mid(pos, cond_end - pos);

		int if_end = condition.indexOf(QLatin1String("!:!"));
		RK_ASSERT(if_end >= 0);
		QString if_part = condition.left(if_end);

		int if_mid = if_part.indexOf(QLatin1String("!=!"));
		RK_ASSERT(if_mid >= 0);
		QString if_compare = if_part.left(if_mid);

		QString if_against = if_part.mid(if_mid + 3);
		if ((if_compare.isEmpty() && if_against.isEmpty()) || (if_compare == if_against)) {
			output = condition.mid(if_end + 3);
			break;
		}

		pos = cond_end + 3;
	} while (pos < max);

	// reached end of template
	commandFinished(output);
}
