/***************************************************************************
                          rkstandardicons  -  description
                             -------------------
    begin                : Wed Oct 24 2007
    copyright            : (C) 2007, 2009, 2010 by Thomas Friedrichsmeier
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
	icons[ActionCDToScript] = KIcon ("folder-txt");

	icons[ActionConfigurePackages] = KIcon ("utilities-file-archiver");

	icons[ActionDeleteRow] = KIcon ("edit-delete");
	icons[ActionInsertRow] = KIcon ("list-add");
	icons[ActionDeleteVar] = icons[ActionDeleteRow];
	icons[ActionInsertVar] = icons[ActionInsertRow];
	icons[ActionPasteInsideTable] = QIcon (rkward_icon_base + "paste_inside_table.png");
	icons[ActionPasteInsideSelection] = QIcon (rkward_icon_base + "paste_inside_selection.png");

	icons[ActionAddRight] = KIcon ("arrow-right");
	icons[ActionRemoveLeft] = KIcon ("arrow-left");

	icons[ActionMoveLeft] = KIcon ("go-previous");
	icons[ActionMoveRight] = KIcon ("go-next");
	icons[ActionMoveFirst] = KIcon ("go-first");
	icons[ActionMoveLast] = KIcon ("go-last");
	icons[ActionMoveUp] = KIcon ("go-up");
	icons[ActionMoveDown] = KIcon ("go-down");

	icons[ActionFlagGreen] = KIcon ("flag-green.png");
	icons[ActionSnapshot] = KIcon ("list-add.png");
	icons[ActionRemovePlot] = KIcon ("list-remove.png");
	icons[ActionWindowDuplicate] = KIcon ("window-duplicate.png");

	icons[ActionClear] = KIcon ("edit-clear.png");

	icons[ActionDetachWindow] = KIcon ("view-fullscreen");
	icons[ActionAttachWindow] = KIcon ("view-restore");

#warning TODO add further action icons here

	// objects
	icons[ObjectList] = QIcon (rkward_icon_base + "list.png");
	icons[ObjectFunction] = QIcon (rkward_icon_base + "function.png");
	icons[ObjectEnvironment] = KIcon ("konqueror");
	icons[ObjectPackageEnvironment] = icons[ActionConfigurePackages];
	icons[ObjectMatrix] = QIcon (rkward_icon_base + "matrix.png");
	icons[ObjectDataFrame] = KIcon ("x-office-spreadsheet");
	icons[ObjectDataNumeric] = QIcon (rkward_icon_base + "data-numeric.png");
	icons[ObjectDataFactor] = QIcon (rkward_icon_base + "data-factor.png");
	icons[ObjectDataCharacter] = KIcon ("draw-text");
	icons[ObjectDataLogical] = QIcon (rkward_icon_base + "data-logical.png");
	icons[ObjectDataUnknown] = KIcon ("unknown");
	icons[ObjectDataOther] = icons[ActionDeleteRow];
	icons[ObjectObjectList] = KIcon ("view-list-tree");

	// windows
	icons[WindowDataFrameEditor] = icons[ObjectDataFrame];
	icons[WindowCommandEditor] = KIcon ("text-x-makefile");	// this may not be the most obvious choice, but it is not quite as awfully close to the data.frame editor icons as most other text icons
	icons[WindowOutput] = KIcon ("xclipboard");
	icons[WindowHelp] = KIcon ("help-contents");
	icons[WindowX11] = KIcon ("x");
	icons[WindowObject] = KIcon ("zoom-original");
	icons[WindowConsole] = KIcon ("utilities-terminal");
	icons[WindowCommandLog] = KIcon ("format-justify-left");
	icons[WindowWorkspaceBrowser] = icons[ObjectObjectList];
	icons[WindowSearchHelp] = KIcon ("help-contents");
	icons[WindowPendingJobs] = KIcon ("system-run");
	icons[WindowFileBrowser] = KIcon ("folder");

/*	This does not work, as the icons are not really Null in this case, but some default icon. Any way to really test this?
	RK_DO ({
		for (int i = ActionRunAll; i < Last; ++i) {
			if (icons[i].isNull ()) qDebug ("Icon %d could not be loaded", i);
		}
	}, MISC, DL_ERROR);
*/
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
	if (object->isType (RObject::Matrix)) return icons[ObjectMatrix];
	if (object->isType (RObject::PackageEnv)) return icons[ObjectPackageEnvironment];
	if (object->isType (RObject::Environment)) return icons[ObjectEnvironment];
	if (object == RObjectList::getObjectList ()) return icons[ObjectObjectList];

	return QIcon ();
}

QIcon RKStandardIcons::iconForWindow (const RKMDIWindow* window) {
	// don't trace this
	if (!window) return QIcon ();

	if (window->isType (RKMDIWindow::DataEditorWindow)) return icons[WindowDataFrameEditor];
	if (window->isType (RKMDIWindow::CommandEditorWindow)) return icons[WindowCommandEditor];
	if (window->isType (RKMDIWindow::OutputWindow)) return icons[WindowOutput];
	if (window->isType (RKMDIWindow::HelpWindow)) return icons[WindowHelp];
	if (window->isType (RKMDIWindow::X11Window)) return icons[WindowX11];
	if (window->isType (RKMDIWindow::ObjectWindow)) return icons[WindowObject];
	if (window->isType (RKMDIWindow::ConsoleWindow)) return icons[WindowConsole];
	if (window->isType (RKMDIWindow::CommandLogWindow)) return icons[WindowCommandLog];
	if (window->isType (RKMDIWindow::WorkspaceBrowserWindow)) return icons[WindowWorkspaceBrowser];
	if (window->isType (RKMDIWindow::SearchHelpWindow)) return icons[WindowSearchHelp];
	if (window->isType (RKMDIWindow::PendingJobsWindow)) return icons[WindowPendingJobs];
	if (window->isType (RKMDIWindow::FileBrowserWindow)) return icons[WindowFileBrowser];

	RK_ASSERT (false);
	return QIcon ();
}
