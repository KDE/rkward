/*
rkstructuregetter - This file is part of RKWard (https://rkward.kde.org). Created: Wed Apr 11 2007
SPDX-FileCopyrightText: 2007-2011 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

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

	meta_attrib = RFn::Rf_install (".rk.meta");
	RFn::Rf_protect (meta_attrib);
	RK_ASSERT (!RFn::Rf_isNull (meta_attrib));

	class_fun = prefetch_fun ("class");
	get_meta_fun = prefetch_fun (".rk.get.meta", false);

// Why do we need all these? Because the is.xxx functions may do an internal dispatch, that we do not want to miss, but don't easily get by e.g. calling RFn::Rf_isFunction() directly.
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

	RFn::Rf_unprotect (num_prefetched_funs + 1); /* all the pre-resolved functions and the meta attribute */
}

SEXP RKStructureGetter::prefetch_fun (const char *name, bool from_base) {
	SEXP ret;

	if (from_base) {
		ret = RFn::Rf_install (name);
	} else {
		ret = RFn::Rf_findFun (RFn::Rf_install (name), ROb(R_GlobalEnv));
	}

	RFn::Rf_protect (ret);
	RK_ASSERT (!RFn::Rf_isNull (ret));
	++num_prefetched_funs;

	return (ret);
}

RData *RKStructureGetter::getStructure (SEXP toplevel, SEXP name, SEXP envlevel, SEXP namespacename) {
	RK_TRACE (RBACKEND);

	QString name_string = RKRSupport::SEXPToString (name);

	// resolve namespace, if needed
	if (RFn::Rf_isNull (namespacename)) {
		with_namespace = false;
	} else {
		SEXP as_ns_fun = RFn::Rf_findFun (RFn::Rf_install (".rk.try.get.namespace"),  ROb(R_GlobalEnv));
		RFn::Rf_protect (as_ns_fun);
		RK_ASSERT (!RFn::Rf_isNull (as_ns_fun));

		namespace_envir = RKRSupport::callSimpleFun (as_ns_fun, namespacename, ROb(R_GlobalEnv));
		with_namespace = !RFn::Rf_isNull (namespace_envir);
		RFn::Rf_unprotect (1);	/* as_ns_fun */
	}

	if (with_namespace) RFn::Rf_protect (namespace_envir);

	RData *ret = new RData;

	toplevel_value = toplevel;
	getStructureSafe (toplevel, name_string, 0, ret, RFn::INTEGER(envlevel)[0]);

	if (with_namespace) RFn::Rf_unprotect (1);	/* namespace_envir */

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

	Rboolean ok = RFn::R_ToplevelExec((void (*)(void*)) getStructureWrapper, &args);

	if (ok != TRUE) {
		storage->discardData();
		RFn::Rf_warning("failure to get object %s", name.toLatin1().data ());
		getStructureWorker(ROb(R_NilValue), name, add_type_flags, storage, nesting_depth);
	}
}

void RKStructureGetter::getStructureWrapper (GetStructureWorkerArgs *data) {
	RK_TRACE (RBACKEND);

	data->getter->getStructureWorker (data->toplevel, data->name, data->add_type_flags, data->storage, data->nesting_depth);
}

/** Temporarily resolve a promise, usually without keeping its value (unless keep_evalled_promises is set, which it never is, at the time of this writing).
 *  This is useful for peeking into large objects while building the object tree, without permanently using lots of RAM.
 *
 *  @note This is is not quite perfect, however. E.g. if we have two promises a and b, where b takes a slice out of a, then
 *        evaluating b will force a, permanently. */
SEXP RKStructureGetter::resolvePromise (SEXP from) {
	RK_TRACE (RBACKEND);

	SEXP ret = from;
	if (RFn::TYPEOF (from) == PROMSXP) {
		ret = RFn::PRVALUE(from);
		if (ret == ROb(R_UnboundValue)) {
			RK_DEBUG(RBACKEND, DL_TRACE, "temporarily resolving unbound promise");

			RFn::Rf_protect(from);
			//SET_PRSEEN(from, 1);  // NOTE: SET_PRSEEN was removed from Rinternals.h in R 4.2.0. Its only use is to prevent recursion, however.
			                        //       Not setting it from here, only means, any recursion will be detected one level later.
			ret = RFn::Rf_eval(RFn::PRCODE(from), RFn::PRENV(from));
			//SET_PRSEEN(from, 0);
			if (keep_evalled_promises) {
				RFn::SET_PRVALUE(from, ret);
				RFn::SET_PRENV(from, ROb(R_NilValue));
			}
			RFn::Rf_unprotect (1);

			RK_DEBUG(RBACKEND, DL_TRACE, "resolved type is %d", RFn::TYPEOF(ret));
		}
	}

	return ret;
}

// TODO: split out some of the large blocks into helper functions, to make this easier to read
void RKStructureGetter::getStructureWorker (SEXP val, const QString &name, int add_type_flags, RData *storage, int nesting_depth) {
	RK_TRACE (RBACKEND);

	bool at_toplevel = (toplevel_value == val);
	bool is_function = false;
	bool is_container = false;
	bool is_environment = false;
	bool no_recurse = (nesting_depth >= 2);	// TODO: should be configurable
	unsigned int type = 0;

	RK_DEBUG(RBACKEND, DL_TRACE, "fetching '%s': %p, s-type %d", name.toLatin1().data(), val, RFn::TYPEOF (val));

	SEXP value = val;
	PROTECT_INDEX value_index;
	RFn::R_ProtectWithIndex (value, &value_index);
	// manually resolve any promises
	RFn::R_Reprotect (value = resolvePromise (value), value_index);

	bool is_s4 = RFn::Rf_isS4 (value);
	SEXP baseenv = ROb(R_BaseEnv);
	if (is_s4) baseenv = ROb(R_GlobalEnv);

	// first field: get name
	RData *namedata = new RData;
	namedata->setData (QStringList (name));

	// get classes
	SEXP classes_s;
	QStringList classes;

	if ((RFn::TYPEOF (value) == LANGSXP) || (RFn::TYPEOF (value) == SYMSXP) || (RFn::TYPEOF (value) == BCODESXP)) {	// if it's a call, we should NEVER send it through eval
		// stripped down and adjusted from R_data_class
		RFn::Rf_protect(classes_s = RFn::Rf_getAttrib(value, ROb(R_ClassSymbol)));
		if (RFn::Rf_length(classes_s)) classes = RKRSupport::SEXPToStringList(classes_s);
		RFn::Rf_unprotect(1);
		if (classes.isEmpty ()) {
			if (RFn::TYPEOF (value) == LANGSXP) {
				SEXP symb = RFn::Rf_protect (RFn::CAR (value));
				QString cl;
				if (RFn::TYPEOF (symb) == SYMSXP) cl = RFn::R_CHAR(RFn::PRINTNAME (symb));
				RFn::Rf_unprotect (1);
				if ((cl != "if") && (cl != "while") && (cl != "for") && (cl != "=") && (cl != "<-") && (cl != "(") && (cl != "{")) cl = "call";
				classes = QStringList (cl);
                        } else if (RFn::TYPEOF (value) == BCODESXP) {
				value = ROb(R_NilValue);   // This is a bit lame, but bytecode cannot be cast to expression, below, and no idea what else to do with it (or what info to extract, anyway)
				classes = QStringList("bytecode");
			} else {
				classes = QStringList ("name");
			}
		}

		RFn::R_Reprotect (value = RFn::Rf_coerceVector (value, EXPRSXP), value_index);	// make sure the object is safe for everything to come
	} else {
		RFn::Rf_protect (classes_s = RKRSupport::callSimpleFun (class_fun, value, baseenv));
		classes = RKRSupport::SEXPToStringList (classes_s);
		RFn::Rf_unprotect (1);
	}

	// store classes
	RData *classdata = new RData;
	classdata->setData (classes);

	// basic classification
	for (int i = classes.size () - 1; i >= 0; --i) {
#ifdef __GNUC__
#	warning: Using is.data.frame() may be more reliable (would need to be called only on List-objects, thus no major performance hit)
#endif
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
			RK_DEBUG (RBACKEND, DL_DEBUG, "Depth limit reached. Will not recurse into %s", name.toLatin1().data ());
		}
	}

	// get meta data, if any
	RData *metadata = new RData;
	if (!RFn::Rf_isNull (RFn::Rf_getAttrib (value, meta_attrib))) {
		SEXP meta_s = RKRSupport::callSimpleFun (get_meta_fun, value, ROb(R_GlobalEnv));
		RFn::Rf_protect (meta_s);
		metadata->setData (RKRSupport::SEXPToStringList (meta_s));
		RFn::Rf_unprotect (1);	/* meta_s */
	} else {
		metadata->setData (QStringList ());
	}

	// get dims
	RData::IntStorage dims;
	SEXP dims_s = RKRSupport::callSimpleFun (dims_fun, value, baseenv);
	if (RFn::Rf_isNumeric (dims_s)) {
		dims = RKRSupport::SEXPToIntArray (dims_s);
	} else {
		unsigned int len = RFn::Rf_length (value);
		if ((len < 2) && (!is_function)) {		// suspicious. Maybe some kind of list
			SEXP len_s = RKRSupport::callSimpleFun (length_fun, value, baseenv);
			RFn::Rf_protect (len_s);
			if (RFn::Rf_isNull (len_s)) {
				dims.append (len);
			} else {
				dims = RKRSupport::SEXPToIntArray (len_s);
			}
			RFn::Rf_unprotect (1); /* len_s */
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
			RK_DEBUG (RBACKEND, DL_DEBUG, "Depth limit reached. Will not recurse into slots of %s", name.toLatin1().data ());
		} else {
			RData::RDataStorage dummy(1, nullptr);
			dummy[0] = new RData ();

			SEXP slots_pseudo_object = RKRSupport::callSimpleFun (rk_get_slots_fun, value, ROb(R_GlobalEnv));
			RFn::Rf_protect (slots_pseudo_object);
			getStructureSafe (slots_pseudo_object, "SLOTS", RObject::PseudoObject, dummy[0], nesting_depth);	// do not increase depth for this pseudo-object
			RFn::Rf_unprotect (1);

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
	RData::RDataStorage res(storage_length, nullptr);
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
			childnames_s = RFn::R_lsInternal3(value, TRUE, FALSE);
		} else if (do_cont) {
			childnames_s = RKRSupport::callSimpleFun (names_fun, value, baseenv);
		} else {
			childnames_s = ROb(R_NilValue); // dummy
		}
		RFn::Rf_protect (childnames_s);
		QStringList childnames = RKRSupport::SEXPToStringList (childnames_s);
		int childcount = childnames.size ();
		if (childcount > NAMED_CHILDREN_LIMIT) {
			RK_DEBUG (RBACKEND, DL_WARNING, "object %s has %d named children. Will only retrieve the first %d", name.toLatin1().data (), childcount, NAMED_CHILDREN_LIMIT);
			childcount = NAMED_CHILDREN_LIMIT;
		}

		RData::RDataStorage children(childcount, nullptr);
		for (int i = 0; i < childcount; ++i) {
			children[i] = new RData ();		// NOTE: RData-ctor pre-initializes these to empty. Thus, we're safe even if there is an error while fetching one of the children.
		}

		if (do_env) {
			RK_DEBUG (RBACKEND, DL_DEBUG, "recurse into environment %s", name.toLatin1().data ());
			if (!RFn::Rf_isEnvironment (value)) {
				// some classes (ReferenceClasses) are identified as environments by is.environment(), but are not internally ENVSXPs.
				// For these, RFn::Rf_findVar would fail.
				RFn::R_Reprotect (value = RKRSupport::callSimpleFun (as_environment_fun, value, ROb(R_GlobalEnv)), value_index);
			}
			for (int i = 0; i < childcount; ++i) {
				SEXP current_childname = RFn::Rf_install(RFn::R_CHAR(RFn::STRING_ELT(childnames_s, i)));		// ??? Why does simply using RFn::STRING_ELT(childnames_i, i) crash?
				RFn::Rf_protect (current_childname);
				SEXP child = RFn::Rf_findVar (current_childname, value);
				RFn::Rf_protect (child);

				getStructureSafe (child, childnames[i], 0, children[i], nesting_depth + 1);
				RFn::Rf_unprotect (2); /* current_childname, child */
			}
		} else if (do_cont) {
			RK_DEBUG (RBACKEND, DL_DEBUG, "recurse into list %s", name.toLatin1().data ());
			// fewer elements than names() can happen, although I doubt it is supposed to happen.
			// see http://sourceforge.net/p/rkward/bugs/67/
			bool may_be_special = RFn::Rf_length (value) < childcount;
			if (RFn::Rf_isList (value) && (!may_be_special)) {		// old style list
				for (int i = 0; i < childcount; ++i) {
					SEXP child = RFn::CAR (value);
					getStructureSafe (child, childnames[i], 0, children[i], nesting_depth + 1);
					RFn::CDR (value);
				}
			} else if (RFn::Rf_isNewList (value) && (!may_be_special)) {				// new style list
				for (int i = 0; i < childcount; ++i) {
					SEXP child = RFn::VECTOR_ELT(value, i);
					getStructureSafe (child, childnames[i], 0, children[i], nesting_depth + 1);
				}
			} else {		// probably an S4 object disguised as a list
				SEXP index = RFn::Rf_allocVector(INTSXP, 1);
				RFn::Rf_protect (index);
				for (int i = 0; i < childcount; ++i) {
					RFn::INTEGER(index)[0] = (i + 1);
					SEXP child = RKRSupport::callSimpleFun2 (double_brackets_fun, value, index, baseenv);
					getStructureSafe (child, childnames[i], 0, children[i], nesting_depth + 1);
				}
				RFn::Rf_unprotect (1); /* index */
			}
		}
		RFn::Rf_unprotect (1);   /* childnames_s */

		RData *childdata = new RData;
		childdata->setData (children);
		res[RObject::StoragePositionChildren] = childdata;

		if (is_environment && at_toplevel && with_namespace) {
			RData *namespacedata = new RData;

			if (no_recurse) {
				type |= RObject::Incomplete;
				RK_DEBUG (RBACKEND, DL_DEBUG, "Depth limit reached. Will not recurse into namespace of %s", name.toLatin1().data ());
			} else {
				RData::RDataStorage dummy(1, nullptr);
				dummy[0] = new RData ();

				getStructureSafe (namespace_envir, "NAMESPACE", RObject::PseudoObject, dummy[0], nesting_depth+99);	// HACK: By default, do not recurse into the children of the namespace, until dealing with the namespace object itself.

				namespacedata->setData (dummy);
			}

			res.insert (RObject::StoragePositionNamespace, namespacedata);
		}
	} else if (is_function) {
// TODO: getting the formals is still a bit of a bottleneck, but no idea, how to improve on this, any further
		SEXP formals_s;
		if (RFn::Rf_isPrimitive (value)) formals_s = RFn::FORMALS (RKRSupport::callSimpleFun (args_fun, value, baseenv));	// primitives don't have formals, internally
		else formals_s = RFn::FORMALS (value);
		RFn::Rf_protect (formals_s);

		// get the default values
		QStringList formals = RKRSupport::SEXPToStringList (formals_s);
		// for the most part, the implicit as.character in SEXPToStringList does a good on the formals (and it's the fastest of many options that I have tried).
		// Only for naked strings (as in 'function (a="something")'), we're missing the quotes. So we add quotes, after conversion, as needed:
		SEXP dummy = formals_s;
		const int formals_len = RFn::Rf_length (formals_s);
		for (int i = 0; i < formals_len; ++i) {
			if (RFn::TYPEOF (RFn::CAR (dummy)) == STRSXP) formals[i] = RKRSharedFunctionality::quote (formals[i]);
			dummy = RFn::CDR (dummy);
		}
		RData *funargvaluesdata = new RData;
		funargvaluesdata->setData (formals);

		// the argument names
		SEXP names_s = RFn::Rf_getAttrib (formals_s, ROb(R_NamesSymbol));
		RFn::Rf_protect (names_s);
		RData *funargsdata = new RData;
		funargsdata->setData (RKRSupport::SEXPToStringList (names_s));

		RFn::Rf_unprotect (2); /* names_s, formals_s */

		res[RObject::StoragePositionFunArgs] = funargsdata;
		res[RObject::StoragePositionFunValues] = funargvaluesdata;
	}

	RFn::Rf_unprotect (1); /* value */

	RK_ASSERT (!res.contains (nullptr));
	storage->setData (res);
}
