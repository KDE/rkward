/***************************************************************************
                          rkstandardicons  -  description
                             -------------------
    begin                : Wed Oct 24 2007
    copyright            : (C) 2007-2018 by Thomas Friedrichsmeier
    email                : thomas.friedrichsmeier@kdemail.net
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

#include "../core/robject.h"
#include "../core/robjectlist.h"
#include "../windows/rkmdiwindow.h"
#include "../misc/rkcommonfunctions.h"

#include "../debug.h"

// static
RKStandardIcons* RKStandardIcons::instance = 0;

void RKStandardIcons::initIcons () {
	RK_TRACE (APP);

	RK_ASSERT (!instance);	// init only once
	instance = new RKStandardIcons ();
	instance->doInitIcons ();
}

// TODO: With number of items growing, we should probably use a lazy-loading approach, instead:
//       if (!loaded[thing]) initIcon (thing);
//       return icons[thing];
void RKStandardIcons::doInitIcons () {
	RK_TRACE (APP);

	// base path for icons provided by rkward itself
	QString rkward_icon_base = RKCommonFunctions::getRKWardDataDir () + "icons/";

	// actions
	icons[ActionRunAll] = QIcon (rkward_icon_base + "run_all.png");
	icons[ActionRunLine] = QIcon (rkward_icon_base + "run_line.png");
	icons[ActionRunSelection] = QIcon (rkward_icon_base + "run_selection.png");
	icons[ActionCDToScript] = QIcon::fromTheme("folder-txt");

	icons[ActionConfigurePackages] = QIcon::fromTheme("utilities-file-archiver");
	icons[ActionConfigureGeneric] = QIcon::fromTheme("configure");
	icons[ActionSearch] = QIcon::fromTheme("edit-find");

	icons[ActionDeleteRow] = QIcon::fromTheme("edit-delete");
	icons[ActionInsertRow] = QIcon::fromTheme("list-add");
	icons[ActionDeleteVar] = icons[ActionDeleteRow];
	icons[ActionInsertVar] = icons[ActionInsertRow];
	icons[ActionPasteInsideTable] = QIcon (rkward_icon_base + "paste_inside_table.png");
	icons[ActionPasteInsideSelection] = QIcon (rkward_icon_base + "paste_inside_selection.png");

	icons[ActionDelete] = icons[ActionDeleteRow];
	icons[ActionAddRight] = QIcon::fromTheme("arrow-right");
	icons[ActionRemoveLeft] = QIcon::fromTheme("arrow-left");

	icons[ActionMoveLeft] = QIcon::fromTheme("go-previous");
	icons[ActionMoveRight] = QIcon::fromTheme("go-next");
	icons[ActionMoveFirst] = QIcon::fromTheme("go-first");
	icons[ActionMoveLast] = QIcon::fromTheme("go-last");
	icons[ActionMoveUp] = QIcon::fromTheme("go-up");
	icons[ActionMoveDown] = QIcon::fromTheme("go-down");

	icons[ActionExpandDown] = QIcon::fromTheme("arrow-right");
	icons[ActionCollapseUp] = QIcon::fromTheme("arrow-down");

	icons[ActionDocumentInfo] = QIcon::fromTheme("documentinfo.png");
	icons[ActionFlagGreen] = QIcon::fromTheme("flag-green.png");
	icons[ActionSnapshot] = QIcon::fromTheme("list-add.png");
	icons[ActionListPlots] = QIcon::fromTheme("view-preview.png");
	icons[ActionRemovePlot] = QIcon::fromTheme("list-remove.png");
	icons[ActionWindowDuplicate] = QIcon::fromTheme("window-duplicate.png");

	icons[ActionClear] = QIcon::fromTheme("edit-clear.png");
	icons[ActionInterrupt] = QIcon::fromTheme("media-playback-stop");

	icons[ActionDetachWindow] = QIcon::fromTheme("view-fullscreen");
	icons[ActionAttachWindow] = QIcon::fromTheme("view-restore");

	icons[ActionLock] = QIcon::fromTheme("object-locked");
	icons[ActionUnlock] = QIcon::fromTheme("object-unlocked");

	icons[ActionShowMenu] = QIcon::fromTheme ("application-menu");
	if (icons[ActionShowMenu].isNull ()) icons[ActionShowMenu] = QIcon (rkward_icon_base + "menu.svg");  // fallback

	// objects
	icons[ObjectList] = QIcon (rkward_icon_base + "list.png");
	icons[ObjectFunction] = QIcon (rkward_icon_base + "function.png");
	icons[ObjectEnvironment] = QIcon::fromTheme("konqueror");
	icons[ObjectPackageEnvironment] = icons[ActionConfigurePackages];
	icons[ObjectMatrix] = QIcon (rkward_icon_base + "matrix.png");
	icons[ObjectDataFrame] = QIcon::fromTheme("x-office-spreadsheet");
	icons[ObjectDataNumeric] = QIcon (rkward_icon_base + "data-numeric.png");
	icons[ObjectDataFactor] = QIcon (rkward_icon_base + "data-factor.png");
	icons[ObjectDataCharacter] = QIcon::fromTheme("draw-text");
	icons[ObjectDataLogical] = QIcon (rkward_icon_base + "data-logical.png");
	icons[ObjectDataUnknown] = QIcon::fromTheme("unknown");
	icons[ObjectDataOther] = icons[ActionDeleteRow];
	icons[ObjectPseudo] = QIcon (rkward_icon_base + "s4_slots.png");

	// windows
	icons[WindowDataFrameEditor] = icons[ObjectDataFrame];
	icons[WindowCommandEditor] = QIcon::fromTheme("text-x-makefile");	// this may not be the most obvious choice, but it is not quite as awfully close to the data.frame editor icons as most other text icons
	icons[WindowOutput] = QIcon::fromTheme("applications-education");
	icons[WindowHelp] = QIcon::fromTheme("help-contents");
	icons[WindowX11] = QIcon::fromTheme("applications-graphics");
	icons[WindowObject] = QIcon::fromTheme("zoom-original");
	icons[WindowConsole] = QIcon::fromTheme("utilities-terminal");
	icons[WindowCommandLog] = QIcon::fromTheme("format-justify-left");
	icons[WindowWorkspaceBrowser] = QIcon::fromTheme("view-list-tree");
	icons[WindowSearchHelp] = QIcon::fromTheme("help-contents");
	icons[WindowPendingJobs] = QIcon::fromTheme("system-run");
	icons[WindowFileBrowser] = QIcon::fromTheme("folder");
	icons[WindowDebugConsole] = QIcon::fromTheme("view-process-system");
	icons[WindowCallstackViewer] = QIcon::fromTheme("view-sort-ascending");

	// TODO: We really want an hourglass symbol, or similar, here.
	icons[StatusWaitingUpdating] = QIcon::fromTheme ("system-search");

	icons[DocumentPDF] = QIcon::fromTheme("application-pdf");

	icons[RKWardIcon] = QIcon::fromTheme("rkward");  // this used to be accessible as QApplication::windowIcon(), but apparently no longer in Qt5

	RK_DO ({
		for (int i = ActionRunAll; i < Last; ++i) {
			if (icons[i].isNull ()) qDebug ("Icon %d could not be loaded", i);
		}
	}, MISC, DL_ERROR);
}

QIcon RKStandardIcons::iconForObject (const RObject* object) {
	// don't trace this

	if (!object) return getIcon (ObjectDataOther);
	if (object->isDataFrame ()) return getIcon (ObjectDataFrame);
	if (object->isVariable()) {
		switch (object->getDataType ()) {
			case RObject::DataNumeric:
				return getIcon (ObjectDataNumeric);
			case RObject::DataFactor:
				return getIcon (ObjectDataFactor);
			case RObject::DataCharacter:
				return getIcon (ObjectDataCharacter);
			case RObject::DataLogical:
				return getIcon (ObjectDataLogical);
			case RObject::DataUnknown:
				return getIcon (ObjectDataUnknown);
			default:
				return getIcon (ObjectDataOther);
		}
	}
	if (object->isSlotsPseudoObject ()) return getIcon (ObjectPseudo);
	if (object->isType (RObject::List)) return getIcon (ObjectList);
	if (object->isType (RObject::Function)) return getIcon (ObjectFunction);
	if (object->isType (RObject::Matrix)) return getIcon (ObjectMatrix);
	if (object->isType (RObject::PackageEnv)) return getIcon (ObjectPackageEnvironment);
	if (object->isType (RObject::Environment)) return getIcon (ObjectEnvironment);

	return QIcon ();
}

QIcon RKStandardIcons::iconForWindow (const RKMDIWindow* window) {
	// don't trace this
	if (!window) return QIcon ();

	if (window->isType (RKMDIWindow::DataEditorWindow)) return getIcon (WindowDataFrameEditor);
	if (window->isType (RKMDIWindow::CommandEditorWindow)) return getIcon (WindowCommandEditor);
	if (window->isType (RKMDIWindow::OutputWindow)) return getIcon (WindowOutput);
	if (window->isType (RKMDIWindow::HelpWindow)) return getIcon (WindowHelp);
	if (window->isType (RKMDIWindow::X11Window)) return getIcon (WindowX11);
	if (window->isType (RKMDIWindow::ObjectWindow)) return getIcon (WindowObject);
	if (window->isType (RKMDIWindow::ConsoleWindow)) return getIcon (WindowConsole);
	if (window->isType (RKMDIWindow::CommandLogWindow)) return getIcon (WindowCommandLog);
	if (window->isType (RKMDIWindow::WorkspaceBrowserWindow)) return getIcon (WindowWorkspaceBrowser);
	if (window->isType (RKMDIWindow::SearchHelpWindow)) return getIcon (WindowSearchHelp);
	if (window->isType (RKMDIWindow::PendingJobsWindow)) return getIcon (WindowPendingJobs);
	if (window->isType (RKMDIWindow::FileBrowserWindow)) return getIcon (WindowFileBrowser);
	if (window->isType (RKMDIWindow::DebugConsoleWindow)) return getIcon (WindowDebugConsole);
	if (window->isType (RKMDIWindow::CallstackViewerWindow)) return getIcon (WindowCallstackViewer);

	RK_ASSERT (false);
	return QIcon ();
}
