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

	class_fun = Rf_findFun (Rf_install ("class"),  R_BaseEnv);;
	PROTECT (class_fun);
	RK_ASSERT (!Rf_isNull (class_fun));

	meta_attrib = Rf_install (".rk.meta");
	PROTECT (meta_attrib);
	RK_ASSERT (!Rf_isNull (meta_attrib));

	get_meta_fun = Rf_findFun (Rf_install (".rk.get.meta"),  R_GlobalEnv);;
	PROTECT (get_meta_fun);
	RK_ASSERT (!Rf_isNull (get_meta_fun));
}

RKStructureGetter::~RKStructureGetter () {
	RK_TRACE (RBACKEND);

	UNPROTECT (3); /* get_meta_fun, meta_attrib, class_fun */
}

RData *RKStructureGetter::getStructure (SEXP toplevel, SEXP name, SEXP namespacename) {
	RK_TRACE (RBACKEND);

	envir_depth = 0;

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

		SEXP as_ns_fun = Rf_findFun (Rf_install ("asNamespace"),  R_BaseEnv);;
		PROTECT (as_ns_fun);
		RK_ASSERT (!Rf_isNull (as_ns_fun));

		SEXP call = allocVector (LANGSXP, 2);
		PROTECT (call);
		SETCAR (call, as_ns_fun);
		SETCAR (CDR (call), namespacename);
		int error;
		SEXP namespace_envir = R_tryEval (call, R_GlobalEnv, &error);
		UNPROTECT (2);	/* as_ns_fun, call */

		if (error) namespace_envir = R_NilValue;

		PROTECT (namespace_envir);
	}

	RData *ret = new RData;
	// TODO: wrap inside a toplevel exec
	getStructureWorker (toplevel, name_string, ret);

	if (with_namespace) {
		UNPROTECT (1);	/* namespace_envir */
	}

	return ret;
}

SEXP RKStructureGetter::resolvePromise (SEXP from) {
	RK_TRACE (RBACKEND);

	SEXP ret = from;

	if (TYPEOF (from) == PROMSXP) {
		if (PRVALUE(from) == R_UnboundValue) {
			PROTECT (from);
			SET_PRSEEN(from, 1);
			ret = eval(PRCODE(from), PRENV(from));
			SET_PRSEEN(from, 0);
			if (keep_evalled_promises) {
				SET_PRVALUE(from, ret);
				SET_PRENV(from, R_NilValue);
			}
			UNPROTECT (1);
		}
	}

	return ret;
}

void RKStructureGetter::getStructureWorker (SEXP val, const QString &name, RData *storage) {
	RK_TRACE (RBACKEND);

	bool is_function = false;
	bool is_container = false;
	bool is_environment = false;
	unsigned int type = 0;
	unsigned int count;
	SEXP call;

	PROTECT (val);
	// manually resolve any promises
	SEXP value = resolvePromise (val);
	PROTECT (value);
	UNPROTECT (1);		/* val */

	//qDebug ("fetching %s", name.latin1());

	// first field: get name
	RData *namedata = new RData;
	namedata->datatype = RData::StringVector;
	namedata->length = 1;
	QString *name_dummy = new QString[1];
	name_dummy[0] = name;
	namedata->data = name_dummy;

	// get classes
	call = allocVector (LANGSXP, 2);
	PROTECT (call);
	SETCAR (call, class_fun);
	SETCAR (CDR (call), value);
	SEXP classes_s = Rf_eval (call, R_GlobalEnv);
	PROTECT (classes_s);
	QString *classes = SEXPToStringList (classes_s, &count);
	unsigned int num_classes = count;
	UNPROTECT (2);	/* classes_s, call */

	// store classes
	RData *classdata = new RData;
	classdata->datatype = RData::StringVector;
	classdata->data = classes;
	classdata->length = num_classes;

	// basic classification
	for (unsigned int i = 0; i < num_classes; ++i) {
		if (classes[i] == "data.frame") type |= RObject::DataFrame;
	}

	if (Rf_isMatrix (value)) type |= RObject::Matrix;
	if (Rf_isArray (value)) type |= RObject::Array;
	if (Rf_isList (value)) type |= RObject::List;
	if (Rf_isNewList (value)) type |= RObject::List;

	if (type != 0) {
		is_container = true;
	} else {
		if (Rf_isFunction (value)) {
			is_function = true;
			type |= RObject::Function;
		} else if (Rf_isEnvironment (value)) {
			is_container = true;
			is_environment = true;
			type |= RObject::Environment;
		} else {
			type |= RObject::Variable;
			if (Rf_isFactor (value)) type |= RObject::Factor;
			else if (Rf_isNumeric (value)) type |= RObject::Numeric;
			else if (Rf_isString (value)) type |= RObject::Character;
			else if (Rf_isLogical (value)) type |= RObject::Logical;
		}
	}

	// TODO: is it misplaced?
	if (with_namespace) {
		// TODO!
		type |= RObject::Misplaced;
	}

	// get meta data, if any
	RData *metadata = new RData;
	metadata->datatype = RData::StringVector;
	if (!Rf_isNull (Rf_getAttrib (value, meta_attrib))) {
		type |= RObject::HasMetaObject;

		call = allocVector (LANGSXP, 2);
		PROTECT (call);
		SETCAR (call, get_meta_fun);
		SETCAR (CDR (call), value);
		SEXP meta_s = Rf_eval (call, R_GlobalEnv);
		PROTECT (meta_s);
		metadata->data = SEXPToStringList (classes_s, &count);
		metadata->length = count;
		UNPROTECT (2);	/* meta_s, call */
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
	SEXP dims_s = Rf_getAttrib (value, R_DimSymbol);
	if (!Rf_isNull (dims_s)) {
		dims = SEXPToIntArray (dims_s, &num_dims);
	} else {
		num_dims = 1;
		dims = new int[1];
		dims[0] = Rf_length (dims_s);
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
		RData *childdata = new RData;
		childdata->datatype = RData::StructureVector;
		childdata->length = 0;
		childdata->data = 0;
		res[5] = childdata;

		if (is_environment) {
			if (++envir_depth > 1) {
			} else {
				//qDebug ("recurse into %s", name.latin1());

				unsigned int childcount;
				SEXP childnames_s = R_lsInternal (value, (Rboolean) 1);
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
				for (unsigned int i = 0; i < childcount; ++i) {
					SEXP child = Rf_findVar (install(CHAR(STRING_ELT(childnames_s, i))), value);
					getStructureWorker (child, childnames[i], children[i]);
				}
				UNPROTECT (1); /* childnames_s */
			}
			--envir_depth;
		} else {
			// TODO: get children from list
		}
	} else if (is_function) {
		RData *funargsdata = new RData;
		funargsdata->datatype = RData::StructureVector;
		funargsdata->length = 0;
		funargsdata->data = 0;
		res[5] = funargsdata;

		RData *funargvaluesdata = new RData;
		funargvaluesdata->datatype = RData::StructureVector;
		funargvaluesdata->length = 0;
		funargvaluesdata->data = 0;
		res[6] = funargvaluesdata;

		// TODO: get and store arguments
	}

	UNPROTECT (1); /* value */
}

