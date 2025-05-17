/*
celleditor - This file is part of the RKWard project. Created: Sat May 17 2025
SPDX-FileCopyrightText: 2025 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKPARSEDSCRIPT_H
#define RKPARSEDSCRIPT_H

#include <QString>

#include <vector>

#include "../debug.h"

/** Very crude, but very fast R parser, with some helper functions for code navigation. Parses the basic structure, only

Technical note on data structure: While, logically, contexts form a nested hierarchy, a nested data layout does not really lend itself
to our purpose, which is to navigate the underlying code, sequentially. So rather, we keep a flat list of contexts, sorted (inherently, during parsing)
by start position.

Inside this flat list, a child context is defined by starting after (or at) the parent's start, and ending before (or at) the parent's end. Child
contexts are always found after their parent in the list.

Type of context. Parenthesis, Brace, and Bracket are the only ContextType s that we actually consider as nested.
*/
class RKParsedScript {
  public:
	enum ContextType {
		None,
		Top,
		Parenthesis,
		Brace,
		Bracket,
		Comment,
		SingleQuoted,
		DoubleQuoted,
		BackQuoted,
		SubsetOperator,
		OtherOperator,
		Delimiter,
		AnySymbol
	};

	struct Context {
		Context(ContextType type, int start) : type(type), start(start) {};
		Context(ContextType type, int start, int end) : type(type), start(start), end(end) {};
		ContextType type;
		int start;
		int end;
	};

	RKParsedScript(const QString &content);

	/** Find the (index of the) innermost context containing pos.
	 *  returns the previous context, if no context actually contains this position (e.g. on a space) */
	int contextAtPos(int pos) const;
	
	const Context &getContext(int index) const {
		return context_list.at(index);
	}

  private:
	// add and parse a context. This is where the actual parsing takes place
	int addContext(ContextType type, int start, const QString &content);

friend class RKCodeNavigation;
	// NOTE: used in debugging, only
	QString serialize() const;
	QString serializeContextEnd(const Context &ctx, int level) const;

	// I want to modify some objects in place during parsing, without triggering copy-on-write
	// hence no Qt container
	std::vector<Context> context_list;
};

#endif
