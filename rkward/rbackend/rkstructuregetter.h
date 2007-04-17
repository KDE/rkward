/***************************************************************************
                          rkstructuregetter  -  description
                             -------------------
    begin                : Wed Apr 11 2007
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

#ifndef RKSTRUCTUREGETTER_H
#define RKSTRUCTUREGETTER_H

/** Low level helper class for getting the structure of R objects (.rk.get.structure).
Since this is only used from REmbedInternal, and making Qt and R includes cooperate is so much trouble,
this is designed to be included directly in rembedinternal.cpp, i.e. includes are not properly defined. I'll fix that later. */
class RKStructureGetter {
public:
	RKStructureGetter (bool keep_evalled_promises);
	~RKStructureGetter ();

	RData *getStructure (SEXP toplevel, SEXP name, SEXP namespacename);
private:
	void getStructureWorker (SEXP value, const QString &name, bool misplaced, RData *storage);
	SEXP resolvePromise (SEXP from);

	SEXP prefetch_fun (char *name, bool from_base=true);

	bool with_namespace;
	SEXP namespace_envir;

	static SEXP callSimpleFun (SEXP fun, SEXP arg, SEXP env);
	static SEXP callSimpleFun2 (SEXP fun, SEXP arg1, SEXP arg2, SEXP env);
	static bool callSimpleBool (SEXP fun, SEXP arg, SEXP env);

	SEXP class_fun;
	SEXP meta_attrib;
	SEXP get_meta_fun;
	SEXP is_matrix_fun;
	SEXP is_array_fun;
	SEXP is_list_fun;
	SEXP is_function_fun;
	SEXP is_environment_fun;
	SEXP is_factor_fun;
	SEXP is_numeric_fun;
	SEXP is_character_fun;
	SEXP is_logical_fun;
	SEXP names_fun;
	SEXP get_formals_fun;
	SEXP double_brackets_fun;
	SEXP length_fun;
	int num_prefetched_funs;

	bool keep_evalled_promises;

	/** current depth of recursion into environments */
	int envir_depth;
};

#endif
