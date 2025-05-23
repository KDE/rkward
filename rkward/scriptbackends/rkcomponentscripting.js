/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
/***************************************************************************
                          rkcomponentscripting  -  description
                             -------------------
    begin                : Thu Jun 17 2010
    copyright            : (C) 2010, 2013 by Thomas Friedrichsmeier
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

// _rkward = Handler object set from RKWard

function Component(id) {
	this._id = id;

	// this one is mostly for internal use
	this.absoluteId = function(id) {
		if (this._id == "") return (id);
		if (id) return (this._id + "." + id);
		return (this._id);
	};

	this.getValue = function(id) {
		return (_rkward.getValue(this.absoluteId(id)));
	};

	this.getString = function(id) {
		return (_rkward.getString(this.absoluteId(id)));
	};

	this.getBoolean = function(id) {
		return (_rkward.getBoolean(this.absoluteId(id)));
	};

	this.getList = function(id) {
		return (_rkward.getList(this.absoluteId(id)));
	};

	this.setValue = function(id, value) {
		return (_rkward.setValue(value, this.absoluteId(id)));
	};

	this.setListValue = function(id, value) {
		return (_rkward.setListValue(value, this.absoluteId(id)));
	};

	this.getChild = function(id) {
		return (new Component(this.absoluteId(id)));
	};

	this.addChangeCommand = function(ids, command) {
		if (Array.isArray(ids)) {
			_rkward.addChangeCommand(ids.map((id) => this.absoluteId(id)), command);
		} else {
			_rkward.addChangeCommand(Array(this.absoluteId(ids)), command);
		}
		return command;
	};
};

makeComponent = function(id) {
	return (new Component(id));
};

gui = new Component("");

doRCommand = function(command, callback) {
	return (_rkward.doRCommand(command, callback));
};

function RCommand(command, id = null) {
	return new Promise(function(resolve, reject) {
		_rkward.doRCommand2(command, id, resolve, reject);
	});
};

function RObject(objectname) {
	this.objectname = objectname;

	// for internal use
	this.initialize = function() {
		info = _rkward.getObjectInfo(this.objectname);

		this._dimensions = info.shift();
		this._classes = info.shift();
		this._isDataFrame = info.shift();
		this._isMatrix = info.shift();
		this._isList = info.shift();
		this._isFunction = info.shift();
		this._isEnvironment = info.shift();
		this._datatype = info.shift();
	};

	this.initialize();

	this.getName = function() {
		return (this.objectname);
	};

	this.exists = function() {
		return (typeof (this._dimensions) != "undefined");
	};

	this.dimensions = function() {
		return (this._dimensions);
	};

	this.classes = function() {
		return (this._classes);
	};

	this.isClass = function(classname) {
		return (this._classes.indexOf(classname) != -1);
	};

	this.isDataFrame = function() {
		return (this._isDataFrame);
	};

	this.isMatrix = function() {
		return (this._isMatrix);
	};

	this.isList = function() {
		return (this._isList);
	};

	this.isFunction = function() {
		return (this._isFunction);
	};

	this.isEnvironment = function() {
		return (this._isEnvironment);
	};

	this.isDataNumeric = function() {
		return (this._datatype == "numeric");
	};

	this.isDataFactor = function() {
		return (this._datatype == "factor");
	};

	this.isDataCharacter = function() {
		return (this._datatype == "character");
	};

	this.isDataLogical = function() {
		return (this._datatype == "logical");
	};

	this.parent = function() {
		return (new RObject(_rkward.getObjectParent(this._name)));
	};

	this.child = function(childname) {
		return (new RObject(_rkward.getObjectChild(this._name, childname)));
	}
};

makeRObject = function(objectname) {
	return (new RObject(objectname));
};

function RObjectArray(names) {
	this._objects = new Array();

	objs = names.split('\n');
	while (objs.count > 0) {
		this._objects.push(new RObject(objs.shift()));
	}
};

makeRObjectArray = function(objectnames) {
	return (new RObjectArray(objectnames));
}
