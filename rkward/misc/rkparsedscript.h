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

Type of context. Parenthesis, Brace, and Bracket, and the outermost context (Top) are the only ContextType s that we actually consider as nested.

-- Addendum/TODO: Wrote that, and implemented it that way, but perhaps it was not the smartes choice after all? The main difficulty is not in
representing the parsed tree, however, but in walking it in the various different ways we want to support. Some things that might help to make some
algos here simpler / faster (should we feel the need):
  - Contexts could store the index of (or a pointer to) their parent -> parentRegion()
  - Parent contexts could store the index of their *last* child -> easier to determine empty regions, to skip over sub-contexts, and to find end points
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
		Context(ContextType type, int start) : type(type), start(start){};
		Context(ContextType type, int start, int end) : type(type), start(start), end(end){};
		bool maybeNesting() const { return (type == Parenthesis || type == Brace || type == Bracket || type == Top); };
		ContextType type;
		int start;
		int end;
	};

	explicit RKParsedScript(const QString &content = QString(), bool rmd = false);

	enum SearchFlags {
		NoFlags,
		SkipInnerContexts = 1,
		SearchBackwards = 2
	};

	/** Helper struct to enforce strict type checking between character position, and index of context */
	struct ContextIndex {
		ContextIndex() : index(-1){};
		explicit ContextIndex(int index) : index(index){};
		bool valid() const { return index >= 0; };
		bool operator==(const ContextIndex &other) const { return index == other.index; };
		bool operator!=(const ContextIndex &other) const { return index != other.index; };
		int index;
	};

	/** Find the innermost context containing pos.
	 *  returns the previous context, if no context actually contains this position (e.g. on a space) */
	ContextIndex contextAtPos(int pos) const;
	ContextIndex nextContext(const ContextIndex from) const;
	ContextIndex prevContext(const ContextIndex from) const;

	/** Find the innermost region (Context::maybeNesting()) containing this context */
	ContextIndex parentRegion(const ContextIndex from) const;

	/** Find next sibling context (no inner or outer contexts) */
	ContextIndex nextSibling(const ContextIndex from) const;
	ContextIndex prevSibling(const ContextIndex from) const;

	ContextIndex nextSiblingOrOuter(const ContextIndex from) const;
	ContextIndex prevSiblingOrOuter(const ContextIndex from) const;

	ContextIndex nextStatement(const ContextIndex from) const;
	ContextIndex prevStatement(const ContextIndex from) const;

	ContextIndex firstContextInStatement(const ContextIndex from) const;
	ContextIndex lastContextInStatement(const ContextIndex from) const;
	int lastPositionInStatement(const ContextIndex from) const;

	ContextIndex nextOuter(const ContextIndex from) const;
	ContextIndex prevOuter(const ContextIndex from) const;

	ContextIndex nextToplevel(const ContextIndex from) const;
	ContextIndex prevToplevel(const ContextIndex from) const;

	ContextIndex nextStatementOrInner(const ContextIndex from) const;
	ContextIndex prevStatementOrInner(const ContextIndex from) const;

	/** Next code Region in R Markdown document */
	ContextIndex nextCodeChunk(const ContextIndex from) const;
	ContextIndex prevCodeChunk(const ContextIndex from) const;
	ContextIndex firstContextInChunk(const ContextIndex from) const;
	int lastPositionInChunk(const ContextIndex from) const;

	/** retrieve the context at the given index. Safe to call, even with an invalid index
	 *  (in which case the outermost context will be returned). */
	const Context &getContext(const ContextIndex index) const {
		if (!index.valid()) return context_list.front();
		return context_list.at(index.index);
	}

  private:
	// add and parse a context. This is where the actual parsing takes place
	int addContext(ContextType type, int start, const QString &content);
	int addNextMarkdownChunk(int start, const QString &content);

	friend class RKParsedScriptTest;
	// NOTE: used in debugging, only
	QString serialize() const;
	QString serializeContextEnd(const Context &ctx, int level) const;

	// I want to modify some objects in place during parsing, without triggering copy-on-write
	// hence no Qt container
	std::vector<Context> context_list;
	ContextType prevtype;
	bool allow_merge;
};

#endif
