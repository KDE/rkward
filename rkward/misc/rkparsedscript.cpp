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

RKParsedScript::RKParsedScript(const QString &content) : prevtype(None), allow_merge(true) {
	context_list.reserve(200); // just a very wild guess
	addContext(Top, -1, content);
	RK_ASSERT(context_list.front().end == content.size() -1);
};

int RKParsedScript::addContext(ContextType type, int start, const QString &content) {
	int index = context_list.size();
	// some contexts need (or benefit from) special handling depending on the preceding context
	// TODO: maybe it would be easier to do this in a separte post-processing step
	if (allow_merge && (type == prevtype) && (type == OtherOperator || type == Delimiter || type == Comment)) {
		// Merge any two subsequent operators or delimiters, and contiguous comment regions into one token
		// i.e. do not add a context, we'll reuse the previous one.
		--index;
	} else if (type == Delimiter && content.at(start) == u'\n' && (prevtype == OtherOperator || prevtype == SubsetOperator)) {
		// newlines do not count as delimiter on operator RHS, so skip ahead, instead of really adding this
		return start;
	} else {
		if (prevtype == Comment) { // comment region implies a newline
			auto pprevtype = context_list.at(index - 2).type;
			if (pprevtype != OtherOperator && pprevtype != SubsetOperator) {
				context_list.emplace_back(Delimiter, start-1, start-1);
				++index;
			}
		}
		context_list.emplace_back(type, start); // end will be filled in, later
	}
	allow_merge = true;

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

	// do not merge comment regions etc across Brace ends
	if (type == Parenthesis || type == Brace || type == Bracket) {
		allow_merge = false;
	}

	// NOTE: we can't just keep a reference to the context at the start of this function, as the vector
	//       may re-allocate during nested parsing
	context_list.at(index).end = pos;
	prevtype = type;
	return pos;
};

RKParsedScript::ContextIndex RKParsedScript::contextAtPos(int pos) const {
	// Context 0 is Top, not really of interest
	for (unsigned int i = 1; i < context_list.size(); ++i) {
		if (context_list.at(i).start > pos) {
			return ContextIndex(i - 1);
		}
	}
	return ContextIndex(0);
}

RKParsedScript::ContextIndex RKParsedScript::nextContext(const ContextIndex from) const {
	int i = from.index + 1;
	if (i >= (int) context_list.size()) i = -1;
	return ContextIndex(i);
}

RKParsedScript::ContextIndex RKParsedScript::prevContext(const ContextIndex from) const {
	int i = from.index - 1;
	if (i >= (int) context_list.size()) i = -1;
	return ContextIndex(i);
}

/** Find the innermost region (Context::maybeNesting()) containing this context */
RKParsedScript::ContextIndex RKParsedScript::parentRegion(const ContextIndex from) const {
	if (from.valid())  {
		int startpos = context_list.at(from.index).start;
		for (int i = from.index-1; i >= 0; --i) {
			if (context_list.at(i).maybeNesting() && context_list.at(i).end >= startpos) {
				return ContextIndex(i);
			}
		}
	}
	return ContextIndex();
}

/** Find next sibling context (no inner or outer contexts) */
RKParsedScript::ContextIndex RKParsedScript::nextSiblingOrOuter(const ContextIndex from) const {
	if (from.valid())  {
		int endpos = context_list.at(from.index).end;
		unsigned int i = from.index;
		do {
			++i;
		} while (i < context_list.size() && context_list.at(i).start <= endpos); // advance, skipping child regions
		return ContextIndex(i < context_list.size() ? i : -1);
	}
	return ContextIndex();
}

RKParsedScript::ContextIndex RKParsedScript::nextSibling(const ContextIndex from) const {
	auto candidate = nextSiblingOrOuter(from);
	if (parentRegion(candidate) == parentRegion(from)) return candidate;
	return ContextIndex();
}

RKParsedScript::ContextIndex RKParsedScript::prevSiblingOrOuter(const ContextIndex from) const {
	if (from.valid())  {
		int startpos = context_list.at(from.index).start;
		int i = from.index;
		do {
			--i;
		} while (i >= 0 && context_list.at(i).end >= startpos); // advance, skipping child regions

		while (true) {
			// handle the case of "a(b)c": Moving rev from c would give us b, however, that's a nephew, not a sibling
			// In this situation we want to return b's parent (i.e. the opening brace)
			// In a loop, as the situation could also be "a{(b)}c", etc.
			auto candidate_parent = parentRegion(ContextIndex(i));
			if (getContext(candidate_parent).end >= startpos) break;
			i = candidate_parent.index;
		}
		return ContextIndex(i);
	}
	return ContextIndex();
}

RKParsedScript::ContextIndex RKParsedScript::prevSibling(const ContextIndex from) const {
	auto candidate = prevSiblingOrOuter(from);
	if (parentRegion(candidate) == parentRegion(from)) return candidate;
	return ContextIndex();
}

RKParsedScript::ContextIndex RKParsedScript::nextStatement(const ContextIndex from) const {
	auto ni = from;
	auto cparent = parentRegion(from);

	// skip over anything that is not an explicit or implicit delimiter
	while (true) {
		ni = nextSiblingOrOuter(ni);
		if (!ni.valid()) break; // hit end
		else if (getContext(ni).type == Delimiter) break;
		else if (getContext(ni).start > getContext(cparent).end) break;
	}

	// skip over any following non-interesting contexts
	while (true) {
		auto type = getContext(ni).type;
		if (type != Delimiter && type != Comment) break;
		ni = nextSiblingOrOuter(ni);
	}
	return ni;
}

RKParsedScript::ContextIndex RKParsedScript::prevStatement(const ContextIndex from) const {
	auto pi = from;

	// TODO: refactor into more readable / reusable parts: statementStart() and statementEnd()

	// find start of current statement (the delimiter, preceding it)
	while (true) {
		pi = prevSiblingOrOuter(pi);
		if (!pi.valid()) break; // hit top end
		else if (getContext(pi).type == Delimiter) break;
	}
	// from here, skip over any preceding non-interesting contexts (entering the previous statement
	while (true) {
		auto type = getContext(pi).type;
		if (type != Delimiter && type != Comment) break;
		pi = prevSiblingOrOuter(pi);
	}
	// now find the start of the previous statement
	while (true) {
		pi = prevSiblingOrOuter(pi);
		if (!pi.valid()) break; // hit top end
		else if (getContext(pi).type == Delimiter) break;
	}
	// actual start is the token after the delimiter
	return nextContext(pi);
}

// NOTE: used in debugging, only
QString RKParsedScript::serialize() const {
	QString ret;
	std::vector<Context> stack;
	stack.push_back(Context(None, -1, INT_MAX)); // dummy context, to avoid empty stack

	for (unsigned int i = 0; i < context_list.size(); ++i) {
		const auto ctx = context_list.at(i);
		qDebug("ctx type %d: %d->%d\n", ctx.type, ctx.start, ctx.end);

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
