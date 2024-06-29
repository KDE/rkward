/*
rkstandardicons - This file is part of the RKWard project. Created: Wed Oct 24 2007
SPDX-FileCopyrightText: 2007-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKSTANDARDICONS_H
#define RKSTANDARDICONS_H

#include <QIcon>

class RObject;
class RKMDIWindow;

/** This class (it's a namespace, really, except I did not figure out how to put the storage array in it as a "private" member) provides easy access to the icons used in RKWard. This helps staying consistent across the whole application.

Also, since QIcons are only loaded on demand, and implicitly shared, this should save some resources for icons that are used often.

Eventually all icons (even those that are only used once, so far) should be added, here. All direct constructions of QIcons (or KIconLoader) should be removed elsewhere.

@author Thomas Friedrichsmeier */
class RKStandardIcons {
public:
	/** initializes the items. */
	static void initIcons ();

	enum IconName {
		ActionRunAll=0,
		ActionRunLine,
		ActionRunSelection,
		ActionCDToScript,

		ActionConfigurePackages,
		ActionConfigureGeneric,
		ActionSearch,

		ActionDeleteRow,
		ActionInsertRow,
		ActionDeleteVar,
		ActionInsertVar,
		ActionPasteInsideTable,
		ActionPasteInsideSelection,

		ActionDelete,
		ActionAddRight,
		ActionRemoveLeft,

		ActionMoveLeft,
		ActionMoveRight,
		ActionMoveFirst,
		ActionMoveLast,
		ActionMoveUp,
		ActionMoveDown,

		ActionExpandDown,
		ActionCollapseUp,

		ActionDocumentInfo,
		ActionFlagGreen,
		ActionSnapshot,
		ActionListPlots,
		ActionRemovePlot,
		ActionWindowDuplicate,

		ActionClear,
		ActionInterrupt,

		ActionDetachWindow,
		ActionAttachWindow,

		ActionLock,
		ActionUnlock,

		ActionShowMenu,
		ActionClose,

		ObjectList,
		ObjectFunction,
		ObjectEnvironment,
		ObjectPackageEnvironment,
		ObjectMatrix,
		ObjectDataFrame,
		ObjectDataNumeric,
		ObjectDataFactor,
		ObjectDataCharacter,
		ObjectDataLogical,
		ObjectDataUnknown,
		ObjectDataOther,
		ObjectPseudo,

		WindowDataFrameEditor,
		WindowCommandEditor,
		WindowOutput,
		WindowHelp,
		WindowX11,
		WindowObject,
		WindowConsole,
		WindowCommandLog,
		WindowWorkspaceBrowser,
		WindowSearchHelp,
		WindowPendingJobs,
		WindowFileBrowser,
		WindowDebugConsole,
		WindowCallstackViewer,
		WindowPDF,

		StatusWaitingUpdating,

		DocumentPDF,

		RKWardIcon,

		Last	/**< not really an item, only the count of items available. Do not use. */
	};

	/** get the icon with the given name */
	static QIcon getIcon (IconName name) { return instance->icons[name]; };

	/** convenience function to get the icon most suited for the given RObject */
	static QIcon iconForObject (const RObject* object);

	/** convenience function to get the icon most suited for the given RKMDIWindow */
	static QIcon iconForWindow (const RKMDIWindow* window);

	/** create a "busy" animation that will cycle through appropriate items using the "setter" function */
	static QTimer* busyAnimation(QObject *parent, std::function<void(const QIcon &)> setter);
private:
	// NOTE: Using a static array of QIcons lead to crashes on exit (Qt 5.4.1). Moving that inside a class instance seems to fix the issue.
	QIcon icons[Last];
	void doInitIcons ();
	static RKStandardIcons* instance;
};

#endif
