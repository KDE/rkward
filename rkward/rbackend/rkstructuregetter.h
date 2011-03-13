/***************************************************************************
                          rkstructuregetter  -  description
                             -------------------
    begin                : Wed Apr 11 2007
    copyright            : (C) 2007, 2009, 2010, 2011 by Thomas Friedrichsmeier
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

#include <Rinternals.h>

#include <QString>

class RData;

/** Low level helper class for getting the structure of R objects (.rk.get.structure). */
class RKStructureGetter {
public:
	RKStructureGetter (bool keep_evalled_promises);
	~RKStructureGetter ();

	RData *getStructure (SEXP toplevel, SEXP name, SEXP envlevel, SEXP namespacename);
private:
	struct GetStructureWorkerArgs {
		SEXP toplevel;
		QString name;
		int add_type_flags;
		RData *storage;
		int nesting_depth;
		RKStructureGetter *getter;
	};

	void getStructureWorker (SEXP value, const QString &name, int add_type_flags, RData *storage, int nesting_depth);
/** needed to wrap things inside an R_ToplevelExec */
	static void getStructureWrapper (GetStructureWorkerArgs *data);
	void getStructureSafe (SEXP value, const QString &name, int add_type_flags, RData *storage, int nesting_depth);
	SEXP resolvePromise (SEXP from);

	SEXP prefetch_fun (const char *name, bool from_base=true);

	bool with_namespace;
	SEXP namespace_envir;

	SEXP class_fun;
	SEXP dims_fun;
	SEXP meta_attrib;
	SEXP get_meta_fun;
	SEXP is_matrix_fun;
	SEXP is_array_fun;
	SEXP is_list_fun;
	SEXP is_function_fun;
	SEXP is_environment_fun;
	SEXP as_environment_fun;
	SEXP is_factor_fun;
	SEXP is_numeric_fun;
	SEXP is_character_fun;
	SEXP is_logical_fun;
	SEXP names_fun;
	SEXP args_fun;
	SEXP double_brackets_fun;
	SEXP length_fun;
	SEXP rk_get_slots_fun;
	int num_prefetched_funs;

	bool keep_evalled_promises;
};

#endif
