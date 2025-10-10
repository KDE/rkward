/*
rkstandardicons - This file is part of RKWard (https://rkward.kde.org). Created: Wed Oct 24 2007
SPDX-FileCopyrightText: 2007-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkstandardicons.h"

#include <QFileInfo>

#include "../core/robject.h"
#include "../core/robjectlist.h"
#include "../misc/rkcommonfunctions.h"
#include "../windows/rkmdiwindow.h"

#include "../debug.h"

// static
RKStandardIcons *RKStandardIcons::instance = nullptr;

void RKStandardIcons::initIcons() {
	RK_TRACE(APP);

	RK_ASSERT(!instance); // init only once
	instance = new RKStandardIcons();
	instance->doInitIcons();
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
void RKStandardIcons::doInitIcons() {
	RK_TRACE(APP);

	// actions
	icons[ActionRunAll] = loadRKWardIcon(QStringLiteral("run_all.png"));
	icons[ActionRunLine] = loadRKWardIcon(QStringLiteral("run_line.png"));
	icons[ActionRunSelection] = loadRKWardIcon(QStringLiteral("run_selection.png"));
	icons[ActionCDToScript] = loadThemeIcon(QStringLiteral("folder-txt"));

	icons[ActionConfigurePackages] = loadThemeIcon(QStringLiteral("utilities-file-archiver"));
	icons[ActionConfigureGeneric] = loadThemeIcon(QStringLiteral("configure"));
	icons[ActionSearch] = loadThemeIcon(QStringLiteral("edit-find"));

	icons[ActionDeleteRow] = loadThemeIcon(QStringLiteral("edit-delete"));
	icons[ActionInsertRow] = loadThemeIcon(QStringLiteral("list-add"));
	icons[ActionDeleteVar] = icons[ActionDeleteRow];
	icons[ActionInsertVar] = icons[ActionInsertRow];
	icons[ActionPasteInsideTable] = loadRKWardIcon(QStringLiteral("paste_inside_table.png"));
	icons[ActionPasteInsideSelection] = loadRKWardIcon(QStringLiteral("paste_inside_selection.png"));

	icons[ActionDelete] = icons[ActionDeleteRow];
	icons[ActionAddRight] = loadThemeIcon(QStringLiteral("arrow-right"));
	icons[ActionRemoveLeft] = loadThemeIcon(QStringLiteral("arrow-left"));

	icons[ActionMoveLeft] = loadThemeIcon(QStringLiteral("go-previous"));
	icons[ActionMoveRight] = loadThemeIcon(QStringLiteral("go-next"));
	icons[ActionMoveFirst] = loadThemeIcon(QStringLiteral("go-first"));
	icons[ActionMoveLast] = loadThemeIcon(QStringLiteral("go-last"));
	icons[ActionMoveUp] = loadThemeIcon(QStringLiteral("go-up"));
	icons[ActionMoveDown] = loadThemeIcon(QStringLiteral("go-down"));

	icons[ActionExpandDown] = loadThemeIcon(QStringLiteral("arrow-right"));
	icons[ActionCollapseUp] = loadThemeIcon(QStringLiteral("arrow-down"));

	icons[ActionDocumentInfo] = loadThemeIcon(QStringLiteral("documentinfo"));
	icons[ActionFlagGreen] = loadThemeIcon(QStringLiteral("flag-green"));
	icons[ActionSnapshot] = loadThemeIcon(QStringLiteral("list-add"));
	icons[ActionListPlots] = loadThemeIcon(QStringLiteral("view-preview"));
	icons[ActionRemovePlot] = loadThemeIcon(QStringLiteral("list-remove"));
	icons[ActionWindowDuplicate] = loadThemeIcon(QStringLiteral("window-duplicate"));

	icons[ActionClear] = loadThemeIcon(QStringLiteral("edit-clear"));
	icons[ActionInterrupt] = loadThemeIcon(QStringLiteral("media-playback-stop"));

	icons[ActionDetachWindow] = loadThemeIcon(QStringLiteral("view-fullscreen"));
	icons[ActionAttachWindow] = loadThemeIcon(QStringLiteral("view-restore"));

	icons[ActionLock] = loadThemeIcon(QStringLiteral("object-locked"));
	icons[ActionUnlock] = loadThemeIcon(QStringLiteral("object-unlocked"));

	icons[ActionShowMenu] = loadThemeIcon(QStringLiteral("application-menu"));
	if (icons[ActionShowMenu].isNull()) icons[ActionShowMenu] = loadRKWardIcon(QStringLiteral("menu.svg")); // fallback
	icons[ActionClose] = loadThemeIcon(QStringLiteral("window-close"));

	// objects
	icons[ObjectList] = loadRKWardIcon(QStringLiteral("list.png"));
	icons[ObjectFunction] = loadRKWardIcon(QStringLiteral("function.png"));
	icons[ObjectEnvironment] = loadThemeIcon(QStringLiteral("konqueror"));
	icons[ObjectPackageEnvironment] = icons[ActionConfigurePackages];
	icons[ObjectMatrix] = loadRKWardIcon(QStringLiteral("matrix.png"));
	icons[ObjectDataFrame] = loadThemeIcon(QStringLiteral("x-office-spreadsheet"));
	icons[ObjectDataNumeric] = loadRKWardIcon(QStringLiteral("data-numeric.png"));
	icons[ObjectDataFactor] = loadRKWardIcon(QStringLiteral("data-factor.png"));
	icons[ObjectDataCharacter] = loadThemeIcon(QStringLiteral("draw-text"));
	icons[ObjectDataLogical] = loadRKWardIcon(QStringLiteral("data-logical.png"));
	icons[ObjectDataUnknown] = loadThemeIcon(QStringLiteral("unknown"));
	icons[ObjectDataOther] = icons[ActionDeleteRow];
	icons[ObjectPseudo] = loadRKWardIcon(QStringLiteral("s4_slots.png"));

	// windows
	icons[WindowDataFrameEditor] = icons[ObjectDataFrame];
	icons[WindowCommandEditor] = loadThemeIcon(QStringLiteral("text-x-makefile")); // this may not be the most obvious choice, but it is not quite as awfully close to the data.frame editor icons as most other text icons
	icons[WindowOutput] = loadThemeIcon(QStringLiteral("applications-education"));
	icons[WindowHelp] = loadThemeIcon(QStringLiteral("help-contents"));
	icons[WindowX11] = loadThemeIcon(QStringLiteral("applications-graphics"));
	icons[WindowObject] = loadThemeIcon(QStringLiteral("zoom-original"));
	icons[WindowConsole] = loadThemeIcon(QStringLiteral("utilities-terminal"));
	icons[WindowCommandLog] = loadThemeIcon(QStringLiteral("format-justify-left"));
	icons[WindowWorkspaceBrowser] = loadThemeIcon(QStringLiteral("view-list-tree"));
	icons[WindowSearchHelp] = loadThemeIcon(QStringLiteral("help-contents"));
	icons[WindowPendingJobs] = loadThemeIcon(QStringLiteral("system-run"));
	icons[WindowFileBrowser] = loadThemeIcon(QStringLiteral("folder"));
	icons[WindowDebugConsole] = loadThemeIcon(QStringLiteral("view-process-system"));
	icons[WindowCallstackViewer] = loadThemeIcon(QStringLiteral("view-sort-ascending"));
	icons[WindowPDF] = loadThemeIcon(QStringLiteral("application-pdf"));

	// TODO: We really want an hourglass symbol, or similar, here.
	icons[StatusWaitingUpdating] = loadThemeIcon(QStringLiteral("system-search"));

	icons[DocumentPDF] = loadThemeIcon(QStringLiteral("application-pdf"));

	// this used to be accessible as QApplication::windowIcon(), but apparently no longer since Qt5 (despite documentation)
	icons[RKWardIcon] = loadRKWardIcon(QStringLiteral("rkward.svgz"));
}

QIcon RKStandardIcons::iconForObject(const RObject *object) {
	// don't trace this

	if (!object) return getIcon(ObjectDataOther);
	if (object->isDataFrame()) return getIcon(ObjectDataFrame);
	if (object->isVariable()) {
		switch (object->getDataType()) {
		case RObject::DataNumeric:
			return getIcon(ObjectDataNumeric);
		case RObject::DataFactor:
			return getIcon(ObjectDataFactor);
		case RObject::DataCharacter:
			return getIcon(ObjectDataCharacter);
		case RObject::DataLogical:
			return getIcon(ObjectDataLogical);
		case RObject::DataUnknown:
			return getIcon(ObjectDataUnknown);
		default:
			return getIcon(ObjectDataOther);
		}
	}
	if (object->isSlotsPseudoObject()) return getIcon(ObjectPseudo);
	if (object->isType(RObject::List)) return getIcon(ObjectList);
	if (object->isType(RObject::Function)) return getIcon(ObjectFunction);
	if (object->isType(RObject::Matrix)) return getIcon(ObjectMatrix);
	if (object->isType(RObject::PackageEnv)) return getIcon(ObjectPackageEnvironment);
	if (object->isType(RObject::Environment)) return getIcon(ObjectEnvironment);

	return QIcon();
}

QIcon RKStandardIcons::iconForWindow(const RKMDIWindow *window) {
	// don't trace this
	if (!window) return QIcon();

	if (window->isType(RKMDIWindow::DataEditorWindow)) return getIcon(WindowDataFrameEditor);
	if (window->isType(RKMDIWindow::CommandEditorWindow)) return getIcon(WindowCommandEditor);
	if (window->isType(RKMDIWindow::OutputWindow)) return getIcon(WindowOutput);
	if (window->isType(RKMDIWindow::HelpWindow)) return getIcon(WindowHelp);
	if (window->isType(RKMDIWindow::X11Window)) return getIcon(WindowX11);
	if (window->isType(RKMDIWindow::ObjectWindow)) return getIcon(WindowObject);
	if (window->isType(RKMDIWindow::ConsoleWindow)) return getIcon(WindowConsole);
	if (window->isType(RKMDIWindow::CommandLogWindow)) return getIcon(WindowCommandLog);
	if (window->isType(RKMDIWindow::WorkspaceBrowserWindow)) return getIcon(WindowWorkspaceBrowser);
	if (window->isType(RKMDIWindow::SearchHelpWindow)) return getIcon(WindowSearchHelp);
	if (window->isType(RKMDIWindow::PendingJobsWindow)) return getIcon(WindowPendingJobs);
	if (window->isType(RKMDIWindow::FileBrowserWindow)) return getIcon(WindowFileBrowser);
	if (window->isType(RKMDIWindow::DebugConsoleWindow)) return getIcon(WindowDebugConsole);
	if (window->isType(RKMDIWindow::CallstackViewerWindow)) return getIcon(WindowCallstackViewer);
	if (window->isType(RKMDIWindow::PDFWindow)) return getIcon(WindowPDF);
	if (window->isType(RKMDIWindow::DebugMessageWindow)) return QIcon();

	RK_ASSERT(false);
	return QIcon();
}

QTimer *RKStandardIcons::busyAnimation(QObject *parent, std::function<void(const QIcon &)> setter) {
	RK_TRACE(APP);

	setter(getIcon(RKWardIcon));
	auto t = new QTimer(parent);
	t->setInterval(750);
	int animation_step = 0;
	QObject::connect(t, &QTimer::timeout, parent, [setter, animation_step]() mutable {
		animation_step = (animation_step + 1) % 2;
		if (animation_step) {
			setter(QIcon::fromTheme(QStringLiteral("computer-symbolic")));
		} else {
			setter(getIcon(RKWardIcon));
		}
	});
	t->start();
	return t;
}
