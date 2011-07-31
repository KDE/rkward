/***************************************************************************
                          rkstandardicons  -  description
                             -------------------
    begin                : Wed Oct 24 2007
    copyright            : (C) 2007, 2009, 2010, 2011 by Thomas Friedrichsmeier
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

#ifndef RKSTANDARDICONS_H
#define RKSTANDARDICONS_H

#include <QIcon>

class RObject;
class RKMDIWindow;

/** This class (it's a namespace, really, except I did not figure out how to put the storage array in it as a "private" member) provides easy access to the icons used in RKWard. This helps staying consistent across the whole application.

Also, since QIcons are only loaded on demand, and implicitely shared, this should save some resources for icons that are used often.

Eventually all icons (even those that are only used once, so far) should be added, here. All direct constructions of QIcons (or KIconLoader) should be removed elsewhere.

TODO: we should also have RKStandardActions

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

		DocumentPDF,

		Last	/**< not really an item, only the count of items available. Do not use. */
	};

	/** get the icon with the given name */
	static QIcon getIcon (IconName name) { return icons[name]; };

	/** convenience function to get the icon most suited for the given RObject */
	static QIcon iconForObject (const RObject* object);

	/** convenience function to get the icon most suited for the given RKMDIWindow */
	static QIcon iconForWindow (const RKMDIWindow* window);
private:
	static QIcon icons[Last];
};

#endif
