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

#include "rkstructuregetter.h"

#include "rdata.h"
#include "rkrsupport.h"
#include "rkrbackend.h"
#include "rkrbackendprotocol_shared.h"
#include "../core/robject.h"

#include "../debug.h"

#define NAMED_CHILDREN_LIMIT 100000

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
	as_environment_fun = prefetch_fun ("as.environment");
	is_factor_fun = prefetch_fun ("is.factor");
	is_numeric_fun = prefetch_fun ("is.numeric");
	is_character_fun = prefetch_fun ("is.character");
	is_logical_fun = prefetch_fun ("is.logical");
	double_brackets_fun = prefetch_fun ("[[");
	dims_fun = prefetch_fun ("dim");
	names_fun = prefetch_fun ("names");
	length_fun = prefetch_fun ("length");
	args_fun = prefetch_fun ("args");
	rk_get_slots_fun = prefetch_fun (".rk.get.slots", false);
}

RKStructureGetter::~RKStructureGetter () {
	RK_TRACE (RBACKEND);

	UNPROTECT (num_prefetched_funs + 1); /* all the pre-resolved functions and the meta attribute */
}

SEXP RKStructureGetter::prefetch_fun (const char *name, bool from_base) {
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

RData *RKStructureGetter::getStructure (SEXP toplevel, SEXP name, SEXP envlevel, SEXP namespacename) {
	RK_TRACE (RBACKEND);

	QString name_string = RKRSupport::SEXPToString (name);

	// resolve namespace, if needed
	if (Rf_isNull (namespacename)) {
		with_namespace = false;
	} else {
		SEXP as_ns_fun = Rf_findFun (Rf_install (".rk.try.get.namespace"),  R_GlobalEnv);
		PROTECT (as_ns_fun);
		RK_ASSERT (!Rf_isNull (as_ns_fun));

		namespace_envir = RKRSupport::callSimpleFun (as_ns_fun, namespacename, R_GlobalEnv);
		with_namespace = !Rf_isNull (namespace_envir);
		UNPROTECT (1);	/* as_ns_fun */
	}

	if (with_namespace) PROTECT (namespace_envir);

	RData *ret = new RData;

	toplevel_value = toplevel;
	getStructureSafe (toplevel, name_string, 0, ret, INTEGER (envlevel)[0]);

	if (with_namespace) UNPROTECT (1);	/* namespace_envir */

	return ret;
}

void RKStructureGetter::getStructureSafe (SEXP value, const QString &name, int add_type_flags, RData *storage, int nesting_depth) {
	RK_TRACE (RBACKEND);

	GetStructureWorkerArgs args;
	args.toplevel = value;
	args.name = name;
	args.add_type_flags = add_type_flags;
	args.storage = storage;
	args.getter = this;
	args.nesting_depth = nesting_depth;

	Rboolean ok = R_ToplevelExec ((void (*)(void*)) getStructureWrapper, &args);

	if (ok != TRUE) {
		storage->discardData();
		Rf_warning ("failure to get object %s", name.toLatin1().data ());
		getStructureWorker (R_NilValue, name, add_type_flags, storage, nesting_depth);
	}
}

void RKStructureGetter::getStructureWrapper (GetStructureWorkerArgs *data) {
	RK_TRACE (RBACKEND);

	data->getter->getStructureWorker (data->toplevel, data->name, data->add_type_flags, data->storage, data->nesting_depth);
}

SEXP RKStructureGetter::resolvePromise (SEXP from) {
	RK_TRACE (RBACKEND);

	SEXP ret = from;
	if (TYPEOF (from) == PROMSXP) {
		ret = PRVALUE(from);
		if (ret == R_UnboundValue) {
			RK_DO (qDebug ("temporarily resolving unbound promise"), RBACKEND, DL_DEBUG);

			PROTECT (from);
			SET_PRSEEN(from, 1);
			ret = Rf_eval(PRCODE(from), PRENV(from));
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
void RKStructureGetter::getStructureWorker (SEXP val, const QString &name, int add_type_flags, RData *storage, int nesting_depth) {
	RK_TRACE (RBACKEND);

	bool at_toplevel = (toplevel_value == val);
	bool is_function = false;
	bool is_container = false;
	bool is_environment = false;
	bool no_recurse = (nesting_depth >= 2);	// TODO: should be configurable
	unsigned int type = 0;

	RK_DO (qDebug ("fetching '%s': %p, s-type %d", name.toLatin1().data(), val, TYPEOF (val)), RBACKEND, DL_DEBUG);

	SEXP value = val;
	PROTECT_INDEX value_index;
	PROTECT_WITH_INDEX (value, &value_index);
	// manually resolve any promises
	REPROTECT (value = resolvePromise (value), value_index);

	bool is_s4 = Rf_isS4 (value);
	SEXP baseenv = R_BaseEnv;
	if (is_s4) baseenv = R_GlobalEnv;

	// first field: get name
	RData *namedata = new RData;
	namedata->setData (QStringList (name));

	// get classes
	SEXP classes_s;

	if ((TYPEOF (value) == LANGSXP) || (TYPEOF (value) == SYMSXP)) {	// if it's a call, we should NEVER send it through eval
		extern SEXP R_data_class (SEXP, Rboolean);
		classes_s = R_data_class (value, (Rboolean) 0);

		REPROTECT (value = Rf_coerceVector (value, EXPRSXP), value_index);	// make sure the object is safe for everything to come

		PROTECT (classes_s);
	} else {
		classes_s = RKRSupport::callSimpleFun (class_fun, value, baseenv);
		PROTECT (classes_s);
	}

	QStringList classes = RKRSupport::SEXPToStringList (classes_s);
	UNPROTECT (1);	/* classes_s */

	// store classes
	RData *classdata = new RData;
	classdata->setData (classes);

	// basic classification
	for (int i = classes.size () - 1; i >= 0; --i) {
#warning: Using is.data.frame() may be more reliable (would need to be called only on List-objects, thus no major performance hit)
		if (classes[i] == "data.frame") type |= RObject::DataFrame;
	}

	if (RKRSupport::callSimpleBool (is_matrix_fun, value, baseenv)) type |= RObject::Matrix;
	if (RKRSupport::callSimpleBool (is_list_fun, value, baseenv)) type |= RObject::List;

	if (type != 0) {
		is_container = true;
		type |= RObject::Container;
	} else {
		if (RKRSupport::callSimpleBool (is_function_fun, value, baseenv)) {
			is_function = true;
			type |= RObject::Function;
		} else if (RKRSupport::callSimpleBool (is_environment_fun, value, baseenv)) {
			is_container = true;
			type |= RObject::Environment;
			is_environment = true;
		} else {
			type |= RObject::Variable;
			if (RKRSupport::callSimpleBool (is_factor_fun, value, baseenv)) type |= RObject::Factor;
			else if (RKRSupport::callSimpleBool (is_numeric_fun, value, baseenv)) type |= RObject::Numeric;
			else if (RKRSupport::callSimpleBool (is_character_fun, value, baseenv)) type |= RObject::Character;
			else if (RKRSupport::callSimpleBool (is_logical_fun, value, baseenv)) type |= RObject::Logical;

			if (RKRSupport::callSimpleBool (is_array_fun, value, baseenv)) type |= RObject::Array;
		}
	}
	type |= add_type_flags;

	if (is_container) {
		if (no_recurse) {
			type |= RObject::Incomplete;
			RK_DO (qDebug ("Depth limit reached. Will not recurse into %s", name.toLatin1().data ()), RBACKEND, DL_DEBUG);
		}
	}

	// get meta data, if any
	RData *metadata = new RData;
	if (!Rf_isNull (Rf_getAttrib (value, meta_attrib))) {
		SEXP meta_s = RKRSupport::callSimpleFun (get_meta_fun, value, R_GlobalEnv);
		PROTECT (meta_s);
		metadata->setData (RKRSupport::SEXPToStringList (meta_s));
		UNPROTECT (1);	/* meta_s */
	} else {
		metadata->setData (QStringList ());
	}

	// get dims
	RData::IntStorage dims;
	SEXP dims_s = RKRSupport::callSimpleFun (dims_fun, value, baseenv);
	if (!Rf_isNull (dims_s)) {
		dims = RKRSupport::SEXPToIntArray (dims_s);
	} else {
		unsigned int len = Rf_length (value);
		if ((len < 2) && (!is_function)) {		// suspicious. Maybe some kind of list
			SEXP len_s = RKRSupport::callSimpleFun (length_fun, value, baseenv);
			PROTECT (len_s);
			if (Rf_isNull (len_s)) {
				dims.append (len);
			} else {
				dims = RKRSupport::SEXPToIntArray (len_s);
			}
			UNPROTECT (1); /* len_s */
		} else {
			dims.append (len);
		}
	}

	// store dims
	RData *dimdata = new RData;
	dimdata->setData (dims);

	RData *slotsdata = new RData ();
	// does it have slots?
	if (is_s4) {
		type |= RObject::S4Object;
		if (no_recurse) {
			type |= RObject::Incomplete;
			RK_DO (qDebug ("Depth limit reached. Will not recurse into slots of %s", name.toLatin1().data ()), RBACKEND, DL_DEBUG);
		} else {
			RData::RDataStorage dummy (1, 0);
			dummy[0] = new RData ();

			SEXP slots_pseudo_object = RKRSupport::callSimpleFun (rk_get_slots_fun, value, R_GlobalEnv);
			PROTECT (slots_pseudo_object);
			getStructureSafe (slots_pseudo_object, "SLOTS", RObject::PseudoObject, dummy[0], nesting_depth);	// do not increase depth for this pseudo-object
			UNPROTECT (1);

			slotsdata->setData (dummy);
		}
	}

	// store type
	RData *typedata = new RData;
	typedata->setData (RData::IntStorage (1, type));

	// store everything we have so far
	int storage_length = RObject::StorageSizeBasicInfo;
	if (is_container) {
		storage_length = RObject::StorageSizeBasicInfo + 1;
	} else if (is_function) {
		storage_length = RObject::StorageSizeBasicInfo + 2;
	}
	RData::RDataStorage res (storage_length, 0);
	res[RObject::StoragePositionName] = namedata;
	res[RObject::StoragePositionType] = typedata;
	res[RObject::StoragePositionClass] = classdata;
	res[RObject::StoragePositionMeta] = metadata;
	res[RObject::StoragePositionDims] = dimdata;
	res[RObject::StoragePositionSlots] = slotsdata;

	// now add the extra info for containers and functions
	if (is_container) {
		bool do_env = (is_environment && (!no_recurse));
		bool do_cont = is_container && (!is_environment) && (!no_recurse);

		// fetch list of child names
		SEXP childnames_s;
		if (do_env) {
			childnames_s = R_lsInternal (value, (Rboolean) 1);
		} else if (do_cont) {
			childnames_s = RKRSupport::callSimpleFun (names_fun, value, baseenv);
		} else {
			childnames_s = R_NilValue; // dummy
		}
		PROTECT (childnames_s);
		QStringList childnames = RKRSupport::SEXPToStringList (childnames_s);
		int childcount = childnames.size ();
		if (childcount > NAMED_CHILDREN_LIMIT) {
			RK_DO (qDebug ("object %s has %d named children. Will only retrieve the first %d", name.toLatin1().data (), childcount, NAMED_CHILDREN_LIMIT), RBACKEND, DL_WARNING);
			childcount = NAMED_CHILDREN_LIMIT;
		}

		RData::RDataStorage children (childcount, 0);
		for (int i = 0; i < childcount; ++i) {
			children[i] = new RData ();		// NOTE: RData-ctor pre-initalizes these to empty. Thus, we're safe even if there is an error while fetching one of the children.
		}

		if (do_env) {
			RK_DO (qDebug ("recurse into environment %s", name.toLatin1().data ()), RBACKEND, DL_DEBUG);
			if (!Rf_isEnvironment (value)) {
				// some classes (ReferenceClasses) are identified as envionments by is.environment(), but are not internally ENVSXPs.
				// For these, Rf_findVar would fail.
				REPROTECT (value = RKRSupport::callSimpleFun (as_environment_fun, value, R_GlobalEnv), value_index);
			}
			for (int i = 0; i < childcount; ++i) {
				SEXP current_childname = Rf_install(CHAR(STRING_ELT(childnames_s, i)));		// ??? Why does simply using STRING_ELT(childnames_i, i) crash?
				PROTECT (current_childname);
				SEXP child = Rf_findVar (current_childname, value);
				PROTECT (child);

				bool child_misplaced = false;
				if (at_toplevel && with_namespace && (!RKRBackend::this_pointer->RRuntimeIsVersion (2, 14, 0))) {
					if (!Rf_isNull (namespace_envir)) {
						SEXP dummy = Rf_findVarInFrame (namespace_envir, current_childname);
						if (Rf_isNull (dummy) || (dummy == R_UnboundValue)) child_misplaced = true;
					}
				}

				getStructureSafe (child, childnames[i], child_misplaced ? RObject::Misplaced : 0, children[i], nesting_depth + 1);
				UNPROTECT (2); /* current_childname, child */
			}
		} else if (do_cont) {
			RK_DO (qDebug ("recurse into list %s", name.toLatin1().data ()), RBACKEND, DL_DEBUG);
			// fewer elements than names() can happen, although I doubt it is supposed to happen.
			// see http://sourceforge.net/tracker/?func=detail&aid=3002439&group_id=50231&atid=459007
			bool may_be_special = Rf_length (value) < childcount;
			if (Rf_isList (value) && (!may_be_special)) {		// old style list
				for (int i = 0; i < childcount; ++i) {
					SEXP child = CAR (value);
					getStructureSafe (child, childnames[i], 0, children[i], nesting_depth + 1);
					CDR (value);
				}
			} else if (Rf_isNewList (value) && (!may_be_special)) {				// new style list
				for (int i = 0; i < childcount; ++i) {
					SEXP child = VECTOR_ELT(value, i);
					getStructureSafe (child, childnames[i], 0, children[i], nesting_depth + 1);
				}
			} else {		// probably an S4 object disguised as a list
				SEXP index = Rf_allocVector(INTSXP, 1);
				PROTECT (index);
				for (int i = 0; i < childcount; ++i) {
					INTEGER (index)[0] = (i + 1);
					SEXP child = RKRSupport::callSimpleFun2 (double_brackets_fun, value, index, baseenv);
					getStructureSafe (child, childnames[i], 0, children[i], nesting_depth + 1);
				}
				UNPROTECT (1); /* index */
			}
		}
		UNPROTECT (1);   /* childnames_s */

		RData *childdata = new RData;
		childdata->setData (children);
		res[RObject::StoragePositionChildren] = childdata;

		if (is_environment && at_toplevel && with_namespace) {
			RData *namespacedata = new RData;

			if (no_recurse) {
				type |= RObject::Incomplete;
				RK_DO (qDebug ("Depth limit reached. Will not recurse into namespace of %s", name.toLatin1().data ()), RBACKEND, DL_DEBUG);
			} else {
				RData::RDataStorage dummy (1, 0);
				dummy[0] = new RData ();

				getStructureSafe (namespace_envir, "NAMESPACE", RObject::PseudoObject, dummy[0], nesting_depth+99);	// HACK: By default, do not recurse into the children of the namespace, until dealing with the namespace object itself.

				namespacedata->setData (dummy);
			}

			res.insert (RObject::StoragePositionNamespace, namespacedata);
		}
	} else if (is_function) {
// TODO: getting the formals is still a bit of a bottleneck, but no idea, how to improve on this, any further
		SEXP formals_s;
		if (Rf_isPrimitive (value)) formals_s = FORMALS (RKRSupport::callSimpleFun (args_fun, value, baseenv));	// primitives don't have formals, internally
		else formals_s = FORMALS (value);
		PROTECT (formals_s);

		// get the default values
		QStringList formals = RKRSupport::SEXPToStringList (formals_s);
		// for the most part, the implicit as.character in SEXPToStringList does a good on the formals (and it's the fastest of many options that I have tried).
		// Only for naked strings (as in 'function (a="something")'), we're missing the quotes. So we add quotes, after conversion, as needed:
		SEXP dummy = formals_s;
		const int formals_len = Rf_length (formals_s);
		for (int i = 0; i < formals_len; ++i) {
			if (TYPEOF (CAR (dummy)) == STRSXP) formals[i] = RKRSharedFunctionality::quote (formals[i]);
			dummy = CDR (dummy);
		}
		RData *funargvaluesdata = new RData;
		funargvaluesdata->setData (formals);

		// the argument names
		SEXP names_s = Rf_getAttrib (formals_s, R_NamesSymbol);
		PROTECT (names_s);
		RData *funargsdata = new RData;
		funargsdata->setData (RKRSupport::SEXPToStringList (names_s));

		UNPROTECT (2); /* names_s, formals_s */

		res[RObject::StoragePositionFunArgs] = funargsdata;
		res[RObject::StoragePositionFunValues] = funargvaluesdata;
	}

	UNPROTECT (1); /* value */

	RK_ASSERT (!res.contains (0));
	storage->setData (res);
}

}	/* extern "C" */
