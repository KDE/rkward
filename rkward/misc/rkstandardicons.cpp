/***************************************************************************
                          rkstandardicons  -  description
                             -------------------
    begin                : Wed Oct 24 2007
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

#include "rkstandardicons.h"

#include "kicon.h"

#include "../core/robject.h"
#include "../core/robjectlist.h"
#include "../windows/rkmdiwindow.h"
#include "../misc/rkcommonfunctions.h"

#include "../debug.h"

// static
QIcon RKStandardIcons::icons[Last];

void RKStandardIcons::initIcons () {
	RK_TRACE (APP);

	RK_ASSERT (icons[ActionRunAll].isNull ());	// init only once

	// base path for icons provided by rkward itself
	QString rkward_icon_base = RKCommonFunctions::getRKWardDataDir () + "icons/";

	// actions
	icons[ActionRunAll] = QIcon (rkward_icon_base + "run_all.png");
	icons[ActionRunLine] = QIcon (rkward_icon_base + "run_line.png");
	icons[ActionRunSelection] = QIcon (rkward_icon_base + "run_selection.png");

	// objects
	icons[ObjectList] = QIcon (rkward_icon_base + "list.png");
	icons[ObjectFunction] = QIcon (rkward_icon_base + "function.png");
#warning KDE 4: some of these icons have been renamed
	icons[ObjectEnvironment] = KIcon ("konqueror");
	icons[ObjectPackageEnvironment] = KIcon ("ark");
	icons[ObjectDataFrame] = KIcon ("spreadsheet");
	icons[ObjectDataNumeric] = KIcon ("math_paren");
	icons[ObjectDataFactor] = KIcon ("math_onetwomatrix");
	icons[ObjectDataCharacter] = KIcon ("text");
#warning TODO icon for logical
	icons[ObjectDataLogical] = QIcon ();
	icons[ObjectDataUnknown] = KIcon ("help");
	icons[ObjectDataOther] = KIcon ("no");
	icons[ObjectObjectList] = KIcon ("view_tree");

	// windows
#warning TODO icons for windows
	icons[WindowDataFrameEditor] = icons[ObjectDataFrame];
}

QIcon RKStandardIcons::iconForObject (const RObject* object) {
	// don't trace this

	if (!object) return icons[ObjectDataOther];
	if (object->isDataFrame ()) return icons[ObjectDataFrame];
	if (object->isVariable()) {
		switch (object->getDataType ()) {
			case RObject::DataNumeric:
				return icons[ObjectDataNumeric];
			case RObject::DataFactor:
				return icons[ObjectDataFactor];
			case RObject::DataCharacter:
				return icons[ObjectDataCharacter];
			case RObject::DataLogical:
				return icons[ObjectDataLogical];
			case RObject::DataUnknown:
				return icons[ObjectDataUnknown];
			default:
				return icons[ObjectDataOther];
		}
	}
	if (object->isType (RObject::List)) return icons[ObjectList];
	if (object->isType (RObject::Function)) return icons[ObjectFunction];
	if (object->isType (RObject::PackageEnv)) return icons[ObjectPackageEnvironment];
	if (object->isType (RObject::Environment)) return icons[ObjectEnvironment];
	if (object == RObjectList::getObjectList ()) return icons[ObjectObjectList];

	return QIcon ();
}

QIcon RKStandardIcons::iconForWindow (const RKMDIWindow* window) {
	// don't trace this
	if (!window) return QIcon ();

	if (window->isType (RKMDIWindow::DataEditorWindow)) return icons[WindowDataFrameEditor];

#warning TODO
	RK_ASSERT (false);
	return QIcon ();
}
