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

RKParsedScript::RKParsedScript(const QString &content, bool rmd) : prevtype(None), allow_merge(true) {
	RK_TRACE(MISC);

	context_list.reserve(200); // just a very wild guess.
	if (rmd) {
		context_list.emplace_back(Top, -1, content.size());
		int i = -1;
		while (i < content.size()) {
			i = addNextMarkdownChunk(i, content);
		}
	} else {
		addContext(Top, -1, content);
		RK_ASSERT(context_list.front().end == content.size());
	}
};

int RKParsedScript::addNextMarkdownChunk(int start, const QString &content) {
	int pos = start;
	int chunkstart = -1;
	QString chunk_barrier;
	while (++pos < content.length()) {
		QChar c = content.at(pos);
		if (c == u'\\') ++pos;
		else if (c == u'`') {
			chunk_barrier = c;
			while (++pos < content.length() && (content.at(pos) == u'`')) {
				chunk_barrier.append(u'`');
			}
			// if there is a leading parameter block, skip that
			if ((pos < content.length()) && content.at(pos) == u'{') {
				while (++pos < content.length()) {
					c = content.at(pos);
					if (c == u'\\') ++pos;
					else if (c == u'}') {
						++pos;
						break;
					}
				}
			}
			chunkstart = pos;
			break;
		}
	}
	if (chunkstart < 0 || chunkstart >= content.size()) {
		context_list.emplace_back(Comment, start, content.size());
		return content.size();
	}
	int chunkend = content.indexOf(chunk_barrier, chunkstart);
	if (chunkend < 0) chunkend = content.size();

	context_list.emplace_back(Comment, start, chunkstart-1);
	prevtype = Comment; // causes insertion of a delimiter in the following addContext
	addContext(Top, chunkstart-1, content.left(chunkend)); // in case markdown region has incomplete syntax
	                                                     // limit parsing to the actual markdown region
	context_list.emplace_back(Top, chunkend, chunkend); // HACK: Used as a dummy separtor, here...
	return chunkend + chunk_barrier.length();
}

int RKParsedScript::addContext(ContextType type, int start, const QString &content) {
	RK_TRACE(MISC);
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
				context_list.emplace_back(Delimiter, start - 1, start - 1);
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
			if (!c.isLetterOrNumber() && c != u'.' && c != u'_') {
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
	if (type == Comment) {
		// following delimiter shall not be considered part of the comment
		context_list.at(index).end = pos - 1;
	} else {
		context_list.at(index).end = pos;
	}
	prevtype = type;
	return pos;
};

RKParsedScript::ContextIndex RKParsedScript::contextAtPos(int pos) const {
	RK_TRACE(MISC);
	// Context 0 is Top, not really of interest
	for (unsigned int i = 1; i < context_list.size(); ++i) {
		if (context_list.at(i).start > pos) {
			return ContextIndex(i - 1);
		}
	}
	return ContextIndex(0);
}

RKParsedScript::ContextIndex RKParsedScript::nextContext(const ContextIndex from) const {
	if (!from.valid()) return ContextIndex();
	if (context_list.at(from.index).type == Top) return ContextIndex();
	int i = from.index + 1;
	if (i >= (int)context_list.size()) i = -1;
	return ContextIndex(i);
}

RKParsedScript::ContextIndex RKParsedScript::prevContext(const ContextIndex from) const {
	if (!from.valid()) return ContextIndex();
	if (context_list.at(from.index).type == Top) return ContextIndex();
	int i = from.index - 1;
	return ContextIndex(i); // automatically invalid, if reversing past 0
}

/** Find the innermost region (Context::maybeNesting()) containing this context */
RKParsedScript::ContextIndex RKParsedScript::parentRegion(const ContextIndex from) const {
	RK_TRACE(MISC);
	if (from.valid()) {
		const int startpos = getContext(from).start;
		auto ci = prevContext(from);
		while (ci.valid()) {
			auto ctx = getContext(ci);
			if (ctx.maybeNesting() && ctx.end >= startpos) {
				return ci;
			}
			ci = prevContext(ci);
		}
	}
	return ContextIndex();
}

/** Find next sibling context (no inner or outer contexts) */
RKParsedScript::ContextIndex RKParsedScript::nextSiblingOrOuter(const ContextIndex from) const {
	RK_TRACE(MISC);
	if (from.valid()) {
		int endpos = context_list.at(from.index).end;
		auto ci = from;
		do {
			ci = nextContext(ci);
		} while (ci.valid() && context_list.at(ci.index).start <= endpos); // advance, skipping child regions
		return ci;
	}
	return ContextIndex();
}

RKParsedScript::ContextIndex RKParsedScript::nextSibling(const ContextIndex from) const {
	RK_TRACE(MISC);
	auto candidate = nextSiblingOrOuter(from);
	if (parentRegion(candidate) == parentRegion(from)) return candidate;
	return ContextIndex();
}

RKParsedScript::ContextIndex RKParsedScript::prevSiblingOrOuter(const ContextIndex from) const {
	RK_TRACE(MISC);
	if (from.valid()) {
		int startpos = context_list.at(from.index).start;
		const auto parent = parentRegion(from);
		auto ci = from;
		while (true) {
			ci = prevContext(ci);
			if (!ci.valid()) break;
			const auto cparent = parentRegion(ci);
			// sibling must have the same parent as us (consider reversing from "c" in "a(b)c"; we shall not stop at b)
			if (cparent == parent) break;
			// but we also allow outer contexts, i.e. parent of previous is a grandparent of us
			if (!cparent.valid()) break;
			if (context_list.at(cparent.index).end >= startpos) break;
		}
		return ci;
	}
	return ContextIndex();
}

RKParsedScript::ContextIndex RKParsedScript::prevSibling(const ContextIndex from) const {
	RK_TRACE(MISC);
	auto candidate = prevSiblingOrOuter(from);
	if (parentRegion(candidate) == parentRegion(from)) return candidate;
	return ContextIndex();
}

RKParsedScript::ContextIndex RKParsedScript::firstContextInStatement(const ContextIndex from) const {
	RK_TRACE(MISC);

	auto pi = from;
	while (true) {
		pi = prevSiblingOrOuter(pi);
		if (!pi.valid()) break; // hit top end
		const auto &pc = getContext(pi);
		if (pc.type == Delimiter) break;
		// Consider reversing from "c" in "a + (b + c)" vs. "a + (b) + c": We want to stop at region
		// openings, too, *if* that region is a parent of this one
		if (pc.maybeNesting() && pc.end > getContext(from).start) break;
	}
	return nextContext(pi);
}

RKParsedScript::ContextIndex RKParsedScript::lastContextInStatement(const ContextIndex from) const {
	RK_TRACE(MISC);

	auto ni = from;
	auto cparent = parentRegion(from);
	// skip over anything that is not an explicit or implicit delimiter
	while (true) {
		ni = nextSiblingOrOuter(ni);
		if (!ni.valid()) break; // hit end
		else if (getContext(ni).type == Delimiter) break;
		else if (getContext(ni).start > getContext(cparent).end) break;
	}
	return prevContext(ni);
}

int RKParsedScript::lastPositionInStatement(const ContextIndex from) const {
	RK_TRACE(MISC);
	auto last = lastContextInStatement(from);
	auto first = firstContextInStatement(from);
	// consider "a <- b({ c })". We want the position at ")", not at "c"
	while (parentRegion(last) != parentRegion(first) && last.valid()) {
		last = parentRegion(last);
	}
	return getContext(last).end;
}

RKParsedScript::ContextIndex RKParsedScript::nextStatement(const ContextIndex from) const {
	RK_TRACE(MISC);

	// forward past end of current statement
	auto ni = nextContext(lastContextInStatement(from));
	// consider advancing from "b" in "a = (b + c) + d; e" -> should be e, not "+ d"
	while (ni.valid() && getContext(ni).type != Delimiter) ni = nextContext(lastContextInStatement(ni));
	// skip over any following non-interesting contexts
	while (ni.valid()) {
		auto type = getContext(ni).type;
		if (type != Delimiter && type != Comment && type != Top) break;
		ni = nextSiblingOrOuter(ni);
	}
	return ni;
}

RKParsedScript::ContextIndex RKParsedScript::prevStatement(const ContextIndex from) const {
	RK_TRACE(MISC);

	// start at the first context preceeding the current statement
	auto pi = prevContext(firstContextInStatement(from));
	// from here, skip over any preceding non-interesting contexts (entering the previous statement)
	while (pi.valid()) {
		auto type = getContext(pi).type;
		if (type != Delimiter && type != Comment && type != Top) break;
		pi = prevSiblingOrOuter(pi);
	}
	// now find the start of the previous statement
	return (firstContextInStatement(pi));
}

RKParsedScript::ContextIndex RKParsedScript::nextOuter(const ContextIndex from) const {
	RK_TRACE(MISC);
	// this may not be a terribly efficient implementation, but a short one:
	auto out = prevOuter(from);
	return nextStatement(out);
}

RKParsedScript::ContextIndex RKParsedScript::prevOuter(const ContextIndex from) const {
	RK_TRACE(MISC);
	return firstContextInStatement(parentRegion(from));
}

RKParsedScript::ContextIndex RKParsedScript::nextToplevel(const ContextIndex from) const {
	RK_TRACE(MISC);
	auto ctx = from;
	auto parent = parentRegion(ctx);
	while (parent.valid() && getContext(parent).type != Top) {
		ctx = prevOuter(ctx);
		parent = parentRegion(ctx);
	}
	return nextStatement(ctx);
}

RKParsedScript::ContextIndex RKParsedScript::prevToplevel(const ContextIndex from) const {
	RK_TRACE(MISC);
	auto ctx = prevStatement(from);
	auto parent = parentRegion(ctx);
	while (parent.valid() && getContext(parent).type != Top) {
		ctx = prevOuter(ctx);
		parent = parentRegion(ctx);
	}
	return firstContextInStatement(ctx);
}

RKParsedScript::ContextIndex RKParsedScript::nextStatementOrInner(const ContextIndex from) const {
	RK_TRACE(MISC);

	auto nsi = nextStatement(from);
	auto ci = nextContext(from);
	while (ci.valid() && (ci.index < nsi.index || !nsi.valid())) {
		auto ctx = getContext(ci);
		// NOTE: We want to move into inner contexts, *unless* they are empty
		if (ctx.maybeNesting()) {
			auto candidate_inner = nextContext(ci);
			if (parentRegion(candidate_inner) == ci) return candidate_inner;
		}
		ci = nextContext(ci);
	}
	return ci;
}

RKParsedScript::ContextIndex RKParsedScript::prevStatementOrInner(const ContextIndex from) const {
	RK_TRACE(MISC);

	auto psi = prevStatement(from); // this is the target, *unless* there is an inner context on the way
	auto parent = parentRegion(from);
	auto ci = prevContext(from);
	while (ci.valid() && ci.index > psi.index) {
		auto cparent = parentRegion(ci);
		if (cparent != parent) {
			// We entered a different parent region. What's the last (inner!) context in it?
			auto candidate = nextContext(cparent);
			while (true) {
				ci = nextStatementOrInner(candidate);
				if (ci.index >= from.index) return candidate;
				candidate = ci;
			}
		}
		ci = prevContext(ci);
	}
	return ci;
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
		for (int j = 0; j < (level - 1) * 4; ++j)
			ret += u" "_s;
		return ret;
	}
	return QString();
}
