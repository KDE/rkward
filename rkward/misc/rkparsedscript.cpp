/*
celleditor - This file is part of the RKWard project. Created: Sat May 17 2025
SPDX-FileCopyrightText: 2025 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rkparsedscript.h"

#include <QChar>

#include <limits.h>

#include "../debug.h"

RKParsedScript::RKParsedScript(const QString &content) {
	context_list.reserve(200); // just a very wild guess
	addContext(Top, -1, content);
};

int RKParsedScript::addContext(ContextType type, int start, const QString &content) {
	ContextType prevtype = context_list.empty() ? None : context_list.back().type;

	int index = context_list.size();
	// some contexts need (or benefit from) special handling depending on the preceding context
	if (type == OtherOperator && prevtype == OtherOperator) {
		// Merge any two subsequent operators into one token
		// i.e. do not add a context, we'll reuse the previous one.
		--index;
	} else if (type == Delimiter && content.at(start) == u'\n' && (prevtype == OtherOperator || prevtype == SubsetOperator)) {
		// newlines do not count as delimiter on operator RHS, so skip ahead, instead of really adding this
		return start;
	} else {
		context_list.emplace_back(type, start); // end will be filled in, later
	}

	int pos = start;
	if (type == SingleQuoted || type == DoubleQuoted || type == BackQuoted) {
		while (++pos < content.length()) {
			const QChar c = content.at(pos);
			if (c == u'\\') ++pos;
			else if (c == u'\'' && type == SingleQuoted) break;
			else if (c == u'"' && type == DoubleQuoted) break;
			else if (c == u'`' && type == BackQuoted) break;
		}
	} else if (type == AnySymbol) {
		while (++pos < content.length()) {
			const QChar c = content.at(pos);
			if (!c.isLetterOrNumber() && c != u'.') {
				--pos;
				break;
			}
		}
	} else if (type == Comment) {
		while (++pos < content.length()) {
			if (content.at(pos) == u'\n') break;
		}
	} else if (type == OtherOperator || type == SubsetOperator || type == Delimiter) {
		// leave context, immediately
	} else {
		while (++pos < content.length()) {
			QChar c = content.at(pos);
			if (c == u'\'') pos = addContext(SingleQuoted, pos, content);
			else if (c == u'"') pos = addContext(DoubleQuoted, pos, content);
			else if (c == u'`') pos = addContext(BackQuoted, pos, content);
			else if (c == u'#') pos = addContext(Comment, pos, content);
			else if (c == u'(') pos = addContext(Parenthesis, pos, content);
			else if (c == u')' && type == Parenthesis) break;
			else if (c == u'{') pos = addContext(Brace, pos, content);
			else if (c == u'}' && type == Brace) break;
			else if (c == u'[') pos = addContext(Bracket, pos, content);
			else if (c == u']' && type == Bracket) break;
			else if (c.isLetterOrNumber() || c == u'.') pos = addContext(AnySymbol, pos, content);
			else if (c == u'\n' || c == u',' || c == u';') pos = addContext(Delimiter, pos, content);
			else if (c == u'$' || c == u'@') pos = addContext(SubsetOperator, pos, content);
			else if (!c.isSpace()) pos = addContext(OtherOperator, pos, content);
		}
	}

	// NOTE: we can't just keep a reference to the context at the start of this function, as the vector
	//       may re-allocate during nested parsing
	context_list.at(index).end = pos;
	return pos;
};

int RKParsedScript::contextAtPos(int pos) const {
	// Context 0 is Top, not really of interest
	for (int i = 1; i < context_list.size(); ++i) {
		if (context_list.at(i).start > pos) {
			return i - 1;
		}
	}
	return 0;
}

// NOTE: used in debugging, only
QString RKParsedScript::serialize() const {
	QString ret;
	std::vector<Context> stack;
	stack.push_back(Context(None, -1, INT_MAX)); // dummy context, to avoid empty stack

	for (unsigned int i = 0; i < context_list.size(); ++i) {
		const auto ctx = context_list.at(i);

		// end any finished contexts
		while (ctx.start >= stack.back().end) {
			ret += serializeContextEnd(stack.back(), stack.size());
			stack.pop_back();
		}

		// now deal with the current context
		stack.push_back(ctx);
		const auto type = ctx.type;
		if (type == Parenthesis) ret += u'(';
		if (type == Brace) ret += u'{';
		if (type == Bracket) ret += u'[';
		if (type == SingleQuoted) ret += u'\'';
		if (type == DoubleQuoted) ret += u'"';
		if (type == BackQuoted) ret += u'`';
		if (type == Comment) ret += u'#';
		if (type == SubsetOperator) ret += u'$';
		if (type == OtherOperator) ret += u'+';
		if (type == AnySymbol) ret += u'x';
	}
	while (!stack.empty()) {
		ret += serializeContextEnd(stack.back(), stack.size());
		stack.pop_back();
	}

	return ret;
}

using namespace Qt::Literals::StringLiterals;

QString RKParsedScript::serializeContextEnd(const Context &ctx, int level) const {
	const auto ptype = ctx.type;

	if (ptype == Parenthesis) return u")"_s;
	if (ptype == Brace) return u"}"_s;
	if (ptype == Bracket) return u"]"_s;
	if (ptype == SingleQuoted) return u"'"_s;
	if (ptype == DoubleQuoted) return u"\""_s;
	if (ptype == BackQuoted) return u"`"_s;
	if (ptype == Comment || ptype == Delimiter) {
		QString ret = u"\n"_s;
		for (int j = 0; j < (level-1) * 4; ++j) ret += u" "_s;
		return ret;
	}
	return QString();
}
