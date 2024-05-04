/*
rkstructuregetter - This file is part of the RKWard project. Created: Wed Apr 11 2007
SPDX-FileCopyrightText: 2007-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKSTRUCTUREGETTER_H
#define RKSTRUCTUREGETTER_H

#include <QString>

#include "rkrapi.h"

class RData;

/** Low level helper class for getting the structure of R objects (.rk.get.structure). */
class RKStructureGetter {
public:
	explicit RKStructureGetter (bool keep_evalled_promises);
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
	SEXP toplevel_value;

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
