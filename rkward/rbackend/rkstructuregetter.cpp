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

	meta_attrib = Rf_install (".rk.meta");
	PROTECT (meta_attrib);
}

RKStructureGetter::~RKStructureGetter () {
	RK_TRACE (RBACKEND);

	UNPROTECT (2); /* meta_attrib, class_fun */
}

RData *RKStructureGetter::getStructure (SEXP toplevel, SEXP name, SEXP namespacename) {
	RK_TRACE (RBACKEND);

	unsigned int count;
	QString *name_dummy = SEXPToStringList (name, &count);
	RK_ASSERT (count == 1);
	QString name_string = name_dummy[0];
	delete [] name_dummy;

	// TODO: resolve namespace, if needed

	RData *ret = new RData;
	// TODO: wrap inside a toplevel exec
	getStructureWorker (toplevel, name_string, /* TODO */ false, ret);

	return ret;


/*
	char *cname = (char*) STRING_PTR (VECTOR_ELT (name, 0));
	SEXP val = findVar (install(CHAR(STRING_ELT(name, 0))), envir);
	if (TYPEOF (val) == PROMSXP) {
		qDebug ("name %s, type %d, unbound %d", cname, TYPEOF (val), PRVALUE(val) == R_UnboundValue);
	} */

}

void RKStructureGetter::getStructureWorker (SEXP value, const QString &name, bool misplaced, RData *storage) {
	RK_TRACE (RBACKEND);

	bool is_function = false;
	bool is_container = false;
	unsigned int type = 0;
	unsigned int count;
	SEXP call;

	// TODO: move all this logic to a separate file
	// TODO: instead of returning an RData, take the parent as parameter, and add to that. Why? Because this way we can tie up all the data earlier. Then, if there is an error (hopefully, there isn't, of course), most memory can be released easily, without the need for much bookkeeping).

	// TODO: make name parameter a const QString&
	// TODO: can namespacename be pre-resolved to a namespace environment? Not that it should matter too much, as it is only needed when recursing into environments, and only once per env.

	// TODO: manually resolve promises

	// first field: get name
	RData *namedata = new RData;
	namedata->datatype = RData::StringVector;
	QString *dummy = new QString[1];
	dummy[0] = name;
	namedata->data = dummy;

	// get classes (note that those are the third element in the RData for historical reasons, but we need to fetch them earlier, in order to find out, whether an object is a data.frame.
	call = allocVector (LANGSXP, 2);
	PROTECT (call);
	SETCAR (call, class_fun);
	SETCAR (CDR (call), value);
	SEXP classes_s = Rf_eval (call, R_GlobalEnv);
	PROTECT (classes_s);
	QString *classes = SEXPToStringList (classes_s, &count);
	unsigned int num_classes = count;
	UNPROTECT (2);	/* classes_s, call */

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
			type |= RObject::Environment;
		} else {
			type |= RObject::Variable;
			if (Rf_isFactor (value)) type |= RObject::Factor;
			else if (Rf_isNumeric (value)) type |= RObject::Numeric;
			else if (Rf_isString (value)) type |= RObject::Character;
			else if (Rf_isLogical (value)) type |= RObject::Logical;
		}
	}
	if (!Rf_isNull (Rf_getAttrib (value, meta_attrib))) type |= RObject::HasMetaObject;
	if (misplaced) type |= RObject::Misplaced;

	// TODO: store type

	// TODO: store classes

	// TODO: get and store dims

	if (is_container) {
		// TODO: get and store children
	} else if (is_function) {
		// TODO: get and store arguments
	}
}

