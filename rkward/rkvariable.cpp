/***************************************************************************
                          rkvariable  -  description
                             -------------------
    begin                : Thu Aug 12 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
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
#include "rkvariable.h"

RKVariable::RKVariable () {
}


RKVariable::~RKVariable () {
}

QString RKVariable::getLabel () {
	return label;
}

QString RKVariable::getShortName () {
	return name;
}

QString RKVariable::getFullName () {
	return ("rk." + table + "[[" + name + "]]");
}

QString RKVariable::getTypeString () {
	return type;
}

QString RKVariable::getDescription () {
	return (name + " (" + label + ")");
}
