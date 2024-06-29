/*
rkstandardicons - This file is part of RKWard (https://rkward.kde.org). Created: Wed Oct 24 2007
SPDX-FileCopyrightText: 2007-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkstandardicons.h"

#include <QFileInfo>

#include "../core/robject.h"
#include "../core/robjectlist.h"
#include "../windows/rkmdiwindow.h"
#include "../misc/rkcommonfunctions.h"

#include "../debug.h"

// static
RKStandardIcons* RKStandardIcons::instance = nullptr;

void RKStandardIcons::initIcons () {
	RK_TRACE (APP);

	RK_ASSERT (!instance);	// init only once
	instance = new RKStandardIcons ();
	instance->doInitIcons ();
}

static QIcon loadThemeIcon(const QString &name) {
	auto ret = QIcon::fromTheme(name);
	if (ret.isNull()) RK_DEBUG(MISC, DL_ERROR, "Theme icon %s could not be loaded", qPrintable(name));
	return ret;
}

static QIcon loadRKWardIcon(const QString &name) {
	// base path for icons provided by rkward itself
	auto ret = QIcon(QStringLiteral(":/rkward/icons/") + name);
	if (ret.isNull()) RK_DEBUG(MISC, DL_ERROR, "Custom icon %s could not be loaded", qPrintable(name));
	return ret;
}

// TODO: With number of items growing, we should probably use a lazy-loading approach, instead:
//       if (!loaded[thing]) initIcon (thing);
//       return icons[thing];
void RKStandardIcons::doInitIcons () {
	RK_TRACE (APP);

	// actions
	icons[ActionRunAll] = loadRKWardIcon("run_all.png");
	icons[ActionRunLine] = loadRKWardIcon("run_line.png");
	icons[ActionRunSelection] = loadRKWardIcon("run_selection.png");
	icons[ActionCDToScript] = loadThemeIcon("folder-txt");

	icons[ActionConfigurePackages] = loadThemeIcon("utilities-file-archiver");
	icons[ActionConfigureGeneric] = loadThemeIcon("configure");
	icons[ActionSearch] = loadThemeIcon("edit-find");

	icons[ActionDeleteRow] = loadThemeIcon("edit-delete");
	icons[ActionInsertRow] = loadThemeIcon("list-add");
	icons[ActionDeleteVar] = icons[ActionDeleteRow];
	icons[ActionInsertVar] = icons[ActionInsertRow];
	icons[ActionPasteInsideTable] = loadRKWardIcon("paste_inside_table.png");
	icons[ActionPasteInsideSelection] = loadRKWardIcon("paste_inside_selection.png");

	icons[ActionDelete] = icons[ActionDeleteRow];
	icons[ActionAddRight] = loadThemeIcon("arrow-right");
	icons[ActionRemoveLeft] = loadThemeIcon("arrow-left");

	icons[ActionMoveLeft] = loadThemeIcon("go-previous");
	icons[ActionMoveRight] = loadThemeIcon("go-next");
	icons[ActionMoveFirst] = loadThemeIcon("go-first");
	icons[ActionMoveLast] = loadThemeIcon("go-last");
	icons[ActionMoveUp] = loadThemeIcon("go-up");
	icons[ActionMoveDown] = loadThemeIcon("go-down");

	icons[ActionExpandDown] = loadThemeIcon("arrow-right");
	icons[ActionCollapseUp] = loadThemeIcon("arrow-down");

	icons[ActionDocumentInfo] = loadThemeIcon("documentinfo");
	icons[ActionFlagGreen] = loadThemeIcon("flag-green");
	icons[ActionSnapshot] = loadThemeIcon("list-add");
	icons[ActionListPlots] = loadThemeIcon("view-preview");
	icons[ActionRemovePlot] = loadThemeIcon("list-remove");
	icons[ActionWindowDuplicate] = loadThemeIcon("window-duplicate");

	icons[ActionClear] = loadThemeIcon("edit-clear");
	icons[ActionInterrupt] = loadThemeIcon("media-playback-stop");

	icons[ActionDetachWindow] = loadThemeIcon("view-fullscreen");
	icons[ActionAttachWindow] = loadThemeIcon("view-restore");

	icons[ActionLock] = loadThemeIcon("object-locked");
	icons[ActionUnlock] = loadThemeIcon("object-unlocked");

	icons[ActionShowMenu] = loadThemeIcon("application-menu");
	if (icons[ActionShowMenu].isNull ()) icons[ActionShowMenu] = loadRKWardIcon("menu.svg");  // fallback
	icons[ActionClose] = loadThemeIcon("window-close");

	// objects
	icons[ObjectList] = loadRKWardIcon("list.png");
	icons[ObjectFunction] = loadRKWardIcon("function.png");
	icons[ObjectEnvironment] = loadThemeIcon("konqueror");
	icons[ObjectPackageEnvironment] = icons[ActionConfigurePackages];
	icons[ObjectMatrix] = loadRKWardIcon("matrix.png");
	icons[ObjectDataFrame] = loadThemeIcon("x-office-spreadsheet");
	icons[ObjectDataNumeric] = loadRKWardIcon("data-numeric.png");
	icons[ObjectDataFactor] = loadRKWardIcon("data-factor.png");
	icons[ObjectDataCharacter] = loadThemeIcon("draw-text");
	icons[ObjectDataLogical] = loadRKWardIcon("data-logical.png");
	icons[ObjectDataUnknown] = loadThemeIcon("unknown");
	icons[ObjectDataOther] = icons[ActionDeleteRow];
	icons[ObjectPseudo] = loadRKWardIcon("s4_slots.png");

	// windows
	icons[WindowDataFrameEditor] = icons[ObjectDataFrame];
	icons[WindowCommandEditor] = loadThemeIcon("text-x-makefile");	// this may not be the most obvious choice, but it is not quite as awfully close to the data.frame editor icons as most other text icons
	icons[WindowOutput] = loadThemeIcon("applications-education");
	icons[WindowHelp] = loadThemeIcon("help-contents");
	icons[WindowX11] = loadThemeIcon("applications-graphics");
	icons[WindowObject] = loadThemeIcon("zoom-original");
	icons[WindowConsole] = loadThemeIcon("utilities-terminal");
	icons[WindowCommandLog] = loadThemeIcon("format-justify-left");
	icons[WindowWorkspaceBrowser] = loadThemeIcon("view-list-tree");
	icons[WindowSearchHelp] = loadThemeIcon("help-contents");
	icons[WindowPendingJobs] = loadThemeIcon("system-run");
	icons[WindowFileBrowser] = loadThemeIcon("folder");
	icons[WindowDebugConsole] = loadThemeIcon("view-process-system");
	icons[WindowCallstackViewer] = loadThemeIcon("view-sort-ascending");
	icons[WindowPDF] = loadThemeIcon("application-pdf");

	// TODO: We really want an hourglass symbol, or similar, here.
	icons[StatusWaitingUpdating] = loadThemeIcon("system-search");

	icons[DocumentPDF] = loadThemeIcon("application-pdf");

	// this used to be accessible as QApplication::windowIcon(), but apparently no longer since Qt5 (despite documentation)
	icons[RKWardIcon] = loadRKWardIcon("rkward.svgz");
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
	if (window->isType(RKMDIWindow::PDFWindow)) return getIcon(WindowPDF);
	if (window->isType (RKMDIWindow::DebugMessageWindow)) return QIcon();

	RK_ASSERT (false);
	return QIcon ();
}

QTimer *RKStandardIcons::busyAnimation(QObject *parent, std::function<void (const QIcon &)> setter) {
	RK_TRACE (APP);

	setter(getIcon(RKWardIcon));
	auto t = new QTimer(parent);
	t->setInterval(750);
	int animation_step = 0;
	QObject::connect(t, &QTimer::timeout, parent, [setter, animation_step]() mutable {
		animation_step = (animation_step + 1) % 2;
		if (animation_step) {
			setter(QIcon::fromTheme("computer-symbolic"));
		} else {
			setter(getIcon(RKWardIcon));
		}
	});
	t->start();
	return t;
}
