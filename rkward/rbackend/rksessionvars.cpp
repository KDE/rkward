/***************************************************************************
                          rksessionvars  -  description
                             -------------------
    begin                : Thu Sep 08 2011
    copyright            : (C) 2011 by Thomas Friedrichsmeier
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

#include "rksessionvars.h"

#include "rinterface.h"

#include "../debug.h"

RKSessionVars* RKSessionVars::_instance = 0;

RKSessionVars::RKSessionVars (RInterface *parent) : QObject (parent) {
	RK_TRACE (RBACKEND);
	RK_ASSERT (!_instance);

	_instance = this;
}

RKSessionVars::~RKSessionVars () {
	RK_TRACE (RBACKEND);
}

void RKSessionVars::setInstalledPackages (const QStringList &new_list) {
	RK_TRACE (RBACKEND);

	installed_packages = new_list;
	emit (installedPackagesChanged ());
}

#include "rksessionvars.moc"
