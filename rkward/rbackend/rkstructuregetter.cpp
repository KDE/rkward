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

#include "rkstructuregetter.h"

RKStructureGetter::RKStructureGetter (bool keep_evalled_promises) {
	RK_TRACE (RBACKEND);

	RKStructureGetter::keep_evalled_promises = keep_evalled_promises;
	num_prefetched_funs = 0;

	meta_attrib = Rf_install (".rk.meta");
	PROTECT (meta_attrib);
	RK_ASSERT (!Rf_isNull (meta_attrib));

	class_fun = prefetch_fun ("class");
	get_meta_fun = prefetch_fun (".rk.get.meta", false);

// Why do we need all these? Because the is.xxx functions may do an internal dispatch, that we do not want to miss, but don't easily get by e.g. calling Rf_isFunction() directly.
	is_matrix_fun = prefetch_fun ("is.matrix");
	is_array_fun = prefetch_fun ("is.array");
	is_list_fun = prefetch_fun ("is.list");
	is_function_fun = prefetch_fun ("is.function");
	is_environment_fun = prefetch_fun ("is.environment");
	is_factor_fun = prefetch_fun ("is.factor");
	is_numeric_fun = prefetch_fun ("is.numeric");
	is_character_fun = prefetch_fun ("is.character");
	is_logical_fun = prefetch_fun ("is.logical");
	double_brackets_fun = prefetch_fun ("[[");
	dims_fun = prefetch_fun ("dim");
	names_fun = prefetch_fun ("names");
	length_fun = prefetch_fun ("length");

	get_formals_fun = prefetch_fun (".rk.get.formals", false);
}

RKStructureGetter::~RKStructureGetter () {
	RK_TRACE (RBACKEND);

	UNPROTECT (num_prefetched_funs + 1); /* all the pre-resolved functions and the meta attribute */
}

SEXP RKStructureGetter::prefetch_fun (CONSTCHAR *name, bool from_base) {
	SEXP ret;

	if (from_base) {
		ret = Rf_install (name);
	} else {
		ret = Rf_findFun (Rf_install (name), R_GlobalEnv);
	}

	PROTECT (ret);
	RK_ASSERT (!Rf_isNull (ret));
	++num_prefetched_funs;

	return (ret);
}

SEXP RKStructureGetter::callSimpleFun (SEXP fun, SEXP arg, SEXP env) {
	SEXP call = allocVector (LANGSXP, 2);
	PROTECT (call);
	SETCAR (call, fun);
	SETCAR (CDR (call), arg);

	SEXP ret = eval (call, env);

	UNPROTECT (1); /* call */
	return ret;
}

SEXP RKStructureGetter::callSimpleFun2 (SEXP fun, SEXP arg1, SEXP arg2, SEXP env) {
	SEXP call = allocVector (LANGSXP, 3);
	PROTECT (call);
	SETCAR (call, fun);
	SETCAR (CDR (call), arg1);
	SETCAR (CDDR (call), arg2);

	SEXP ret = eval (call, env);

	UNPROTECT (1); /* call */
	return ret;
}

bool RKStructureGetter::callSimpleBool (SEXP fun, SEXP arg, SEXP env) {
	SEXP res = callSimpleFun (fun, arg, env);
	RK_ASSERT (TYPEOF (res) == LGLSXP);
	return ((bool) LOGICAL (res)[0]);
}

RData *RKStructureGetter::getStructure (SEXP toplevel, SEXP name, SEXP envlevel, SEXP namespacename) {
	RK_TRACE (RBACKEND);

	// TODO: accept an envlevel parameter
	envir_depth = INTEGER (envlevel)[0];

	unsigned int count;
	QString *name_dummy = SEXPToStringList (name, &count);
	RK_ASSERT (count == 1);
	QString name_string = name_dummy[0];
	delete [] name_dummy;

	// resolve namespace, if needed
	if (Rf_isNull (namespacename)) {
		with_namespace = false;
	} else {
		with_namespace = true;

		SEXP as_ns_fun = Rf_findFun (Rf_install (".rk.try.get.namespace"),  R_GlobalEnv);
		PROTECT (as_ns_fun);
		RK_ASSERT (!Rf_isNull (as_ns_fun));

		namespace_envir = callSimpleFun (as_ns_fun, namespacename, R_GlobalEnv);
		UNPROTECT (1);	/* as_ns_fun */

		PROTECT (namespace_envir);
	}

	RData *ret = new RData;

	getStructureSafe (toplevel, name_string, false, ret);

	if (with_namespace) {
		UNPROTECT (1);	/* namespace_envir */
	}

	return ret;
}

void RKStructureGetter::getStructureSafe (SEXP value, const QString &name, bool misplaced, RData *storage) {
	RK_TRACE (RBACKEND);

	GetStructureWorkerArgs args;
	args.toplevel = value;
	args.name = name;
	args.misplaced = misplaced;
	args.storage = storage;
	args.getter = this;

	Rboolean ok = R_ToplevelExec ((void (*)(void*)) getStructureWrapper, &args);

	if (ok != TRUE) {
		storage->discardData();
		Rf_warning ("failure to get object %s", name.toLatin1().data ());
		getStructureWorker (R_NilValue, name, misplaced, storage);
	}
}

void RKStructureGetter::getStructureWrapper (GetStructureWorkerArgs *data) {
	RK_TRACE (RBACKEND);

	data->getter->getStructureWorker (data->toplevel, data->name, data->misplaced, data->storage);
}

SEXP RKStructureGetter::resolvePromise (SEXP from) {
	RK_TRACE (RBACKEND);

	SEXP ret = from;
	if (TYPEOF (from) == PROMSXP) {
		if (PRVALUE(from) == R_UnboundValue) {
			RK_DO (qDebug ("temporarily resolving unbound promise"), RBACKEND, DL_DEBUG);

			PROTECT (from);
			SET_PRSEEN(from, 1);
			ret = eval(PRCODE(from), PRENV(from));
			SET_PRSEEN(from, 0);
			if (keep_evalled_promises) {
				SET_PRVALUE(from, ret);
				SET_PRENV(from, R_NilValue);
			}
			UNPROTECT (1);

			RK_DO (qDebug ("resolved type is %d", TYPEOF (ret)), RBACKEND, DL_DEBUG);
		}
	}

	return ret;
}

extern "C" {
// TODO: split out some of the large blocks into helper functions, to make this easier to read
void RKStructureGetter::getStructureWorker (SEXP val, const QString &name, bool misplaced, RData *storage) {
	RK_TRACE (RBACKEND);

	bool is_function = false;
	bool is_container = false;
	bool is_environment = false;
	unsigned int type = 0;
	unsigned int count;

	RK_DO (qDebug ("fetching '%s': %p, s-type %d", name.toLatin1().data(), val, TYPEOF (val)), RBACKEND, DL_DEBUG);

	PROTECT (val);
	// manually resolve any promises
	SEXP value = resolvePromise (val);
	UNPROTECT (1);		/* val */
	PROTECT (value);

	// first field: get name
	RData *namedata = new RData;
	namedata->datatype = RData::StringVector;
	namedata->length = 1;
	QString *name_dummy = new QString[1];
	name_dummy[0] = name;
	namedata->data = name_dummy;

	// get classes
	SEXP classes_s;

	if (TYPEOF (value) == LANGSXP) {	// if it's a call, we should NEVER send it through eval
		extern SEXP R_data_class (SEXP, Rboolean);
		classes_s = R_data_class (value, (Rboolean) 0);

		value = coerceVector (value, EXPRSXP);	// make sure the object is safe for everything to come
		UNPROTECT (1); /* old value */

		PROTECT (classes_s);
		PROTECT (value);
	} else {
		classes_s = callSimpleFun (class_fun, value, R_BaseEnv);
		PROTECT (classes_s);
	}

	QString *classes = SEXPToStringList (classes_s, &count);
	unsigned int num_classes = count;
	UNPROTECT (1);	/* classes_s */

	// store classes
	RData *classdata = new RData;
	classdata->datatype = RData::StringVector;
	classdata->data = classes;
	classdata->length = num_classes;

	// basic classification
	for (unsigned int i = 0; i < num_classes; ++i) {
		if (classes[i] == "data.frame") type |= RObject::DataFrame;
	}

	if (callSimpleBool (is_matrix_fun, value, R_BaseEnv)) type |= RObject::Matrix;
	if (callSimpleBool (is_array_fun, value, R_BaseEnv)) type |= RObject::Array;
	if (callSimpleBool (is_list_fun, value, R_BaseEnv)) type |= RObject::List;

	if (type != 0) {
		is_container = true;
		type |= RObject::Container;
	} else {
		if (callSimpleBool (is_function_fun, value, R_BaseEnv)) {
			is_function = true;
			type |= RObject::Function;
		} else if (callSimpleBool (is_environment_fun, value, R_BaseEnv)) {
			is_container = true;
			is_environment = true;
			type |= RObject::Environment;
		} else {
			type |= RObject::Variable;
			if (callSimpleBool (is_factor_fun, value, R_BaseEnv)) type |= RObject::Factor;
			else if (callSimpleBool (is_numeric_fun, value, R_BaseEnv)) type |= RObject::Numeric;
			else if (callSimpleBool (is_character_fun, value, R_BaseEnv)) type |= RObject::Character;
			else if (callSimpleBool (is_logical_fun, value, R_BaseEnv)) type |= RObject::Logical;
		}
	}
	if (misplaced) type |= RObject::Misplaced;

	// get meta data, if any
	RData *metadata = new RData;
	metadata->datatype = RData::StringVector;
	if (!Rf_isNull (Rf_getAttrib (value, meta_attrib))) {
		type |= RObject::HasMetaObject;

		SEXP meta_s = callSimpleFun (get_meta_fun, value, R_GlobalEnv);
		PROTECT (meta_s);
		metadata->data = SEXPToStringList (meta_s, &count);
		metadata->length = count;
		UNPROTECT (1);	/* meta_s */
	} else {
		metadata->length = 1;
		QString *meta_dummy = new QString[1];
		meta_dummy[0] = "";
		metadata->data = meta_dummy;
	}

	// store type
	RData *typedata = new RData;
	typedata->datatype = RData::IntVector;
	typedata->length = 1;
	int *type_dummy = new int[1];
	type_dummy[0] = type;
	typedata->data = type_dummy;

	// get dims
	int *dims;
	unsigned int num_dims;
	SEXP dims_s = callSimpleFun (dims_fun, value, R_BaseEnv);
	if (!Rf_isNull (dims_s)) {
		dims = SEXPToIntArray (dims_s, &num_dims);
	} else {
		num_dims = 1;

		unsigned int len = Rf_length (value);
		if ((len < 2) && (!is_function)) {		// suspicious. Maybe some kind of list
			SEXP len_s = callSimpleFun (length_fun, value, R_BaseEnv);
			PROTECT (len_s);
			if (Rf_isNull (len_s)) {
				dims = new int[1];
				dims[0] = len;
			} else {
				dims = SEXPToIntArray (len_s, &num_dims);
			}
			UNPROTECT (1); /* len_s */
		} else {
			dims = new int[1];
			dims[0] = len;
		}
	}

	// store dims
	RData *dimdata = new RData;
	dimdata->datatype = RData::IntVector;
	dimdata->length = num_dims;
	dimdata->data = dims;

	// store everything we have so far
	if (is_container) {
		storage->length = 6;
	} else if (is_function) {
		storage->length = 7;
	} else {
		storage->length = 5;
	}
	storage->datatype = RData::StructureVector;
	RData **res = new RData*[storage->length];
	storage->data = res;
	res[0] = namedata;
	res[1] = typedata;
	res[2] = classdata;
	res[3] = metadata;
	res[4] = dimdata;

	// now add the extra info for containers and functions
	if (is_container) {
		bool do_env = (is_environment && (++envir_depth < 2));
		bool do_cont = is_container && (!is_environment);

		RData *childdata = new RData;
		childdata->datatype = RData::StructureVector;
		childdata->length = 0;
		childdata->data = 0;
		res[5] = childdata;

		// fetch list of child names
		unsigned int childcount;
		SEXP childnames_s;
		if (do_env) {
			childnames_s = R_lsInternal (value, (Rboolean) 1);
		} else if (do_cont) {
			childnames_s = callSimpleFun (names_fun, value, R_BaseEnv);
		} else {
			childnames_s = R_NilValue; // dummy
		}
		PROTECT (childnames_s);
		QString *childnames = SEXPToStringList (childnames_s, &childcount);

		childdata->length = childcount;
		RData **children = new RData*[childcount];
		childdata->data = children;
		childdata->length = childcount;
		for (unsigned int i = 0; i < childcount; ++i) {		// in case there is an error while fetching one of the children, let's pre-initialize everything.
			children[i] = new RData;
			children[i]->data = 0;
			children[i]->length = 0;
			children[i]->datatype = RData::NoData;
		}

		if (do_env) {
			RK_DO (qDebug ("recurse into environment %s", name.toLatin1().data ()), RBACKEND, DL_DEBUG);
			for (unsigned int i = 0; i < childcount; ++i) {
				SEXP current_childname = install(CHAR(STRING_ELT(childnames_s, i)));
				PROTECT (current_childname);
				SEXP child = Rf_findVar (current_childname, value);
				PROTECT (child);

				bool child_misplaced = false;
				if (with_namespace) {
					/* before R 2.4.0, operator "::" would only work on true namespaces, not on package names (operator "::" work, if there is a namespace, and that namespace has the symbol in it)
					TODO remove once we depend on R >= 2.4.0 */
#					ifndef R_2_5
					if (Rf_isNull (namespace_envir)) {
						child_misplaced = true;
					} else {
						SEXP dummy = Rf_findVarInFrame (namespace_envir, current_childname);
						if (Rf_isNull (dummy) || (dummy == R_UnboundValue)) child_misplaced = true;
					}
					/* for R 2.4.0 or greater: operator "::" works if package has no namespace at all, or has a namespace with the symbol in it */
#					else
					if (!Rf_isNull (namespace_envir)) {
						SEXP dummy = Rf_findVarInFrame (namespace_envir, current_childname);
						if (Rf_isNull (dummy) || (dummy == R_UnboundValue)) child_misplaced = true;
					}
#					endif
				}

				getStructureSafe (child, childnames[i], child_misplaced, children[i]);
				UNPROTECT (2); /* childname, child */
			}
		} else if (do_cont) {
			RK_DO (qDebug ("recurse into list %s", name.toLatin1().data ()), RBACKEND, DL_DEBUG);
			if (Rf_isList (value) && (!Rf_isObject (value))) {		// old style list
				for (unsigned int i = 0; i < childcount; ++i) {
					SEXP child = CAR (value);
					getStructureSafe (child, childnames[i], false, children[i]);
					CDR (value);
				}
			} else if (Rf_isNewList (value) && (!Rf_isObject (value))) {				// new style list
				for (unsigned int i = 0; i < childcount; ++i) {
					SEXP child = VECTOR_ELT(value, i);
					getStructureSafe (child, childnames[i], false, children[i]);
				}
			} else {		// probably an S4 object disguised as a list
				SEXP index = Rf_allocVector(INTSXP, 1);
				PROTECT (index);
				for (unsigned int i = 0; i < childcount; ++i) {
					INTEGER (index)[0] = (i + 1);
					SEXP child = callSimpleFun2 (double_brackets_fun, value, index, R_BaseEnv);
					getStructureSafe (child, childnames[i], false, children[i]);
				}
				UNPROTECT (1); /* index */
			}
		}
		UNPROTECT (1);   /* childnames_s */
	} else if (is_function) {
		RData *funargsdata = new RData;
		funargsdata->datatype = RData::StringVector;
		funargsdata->length = 0;
		funargsdata->data = 0;
		res[5] = funargsdata;

		RData *funargvaluesdata = new RData;
		funargvaluesdata->datatype = RData::StringVector;
		funargvaluesdata->length = 0;
		funargvaluesdata->data = 0;
		res[6] = funargvaluesdata;

// TODO: this is still the major bottleneck, but no idea, how to improve on this
		SEXP formals_s = callSimpleFun (get_formals_fun, value, R_GlobalEnv);
		PROTECT (formals_s);
		// the default values
		funargvaluesdata->data = SEXPToStringList (formals_s, &(funargvaluesdata->length));

		// the argument names
		SEXP names_s = getAttrib (formals_s, R_NamesSymbol);
		PROTECT (names_s);
		funargsdata->data = SEXPToStringList (names_s, &(funargsdata->length));

		UNPROTECT (2); /* names_s, formals_s */
	}

	UNPROTECT (1); /* value */
}

}	/* extern "C" */
