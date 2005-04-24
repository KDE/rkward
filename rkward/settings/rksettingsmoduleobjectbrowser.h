/***************************************************************************
                          rksettingsmodule  -  description
                             -------------------
    begin                : Fri Apr 22 2005
    copyright            : (C) 2005 by Thomas Friedrichsmeier
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

#ifndef RKSETTINGSMODULEOBJECTBROWSER_H
#define RKSETTINGSMODULEOBJECTBROWSER_H

#include "rksettingsmodule.h"

class QCheckBox;

/** Configuration module for instances of RObjectListView
@see RKSettingsModule
@author Thomas Friedrichsmeier
*/
class RKSettingsModuleObjectBrowser : public RKSettingsModule {
	Q_OBJECT
public:
	RKSettingsModuleObjectBrowser (RKSettings *gui, QWidget *parent);

	~RKSettingsModuleObjectBrowser ();

/** @returns whether changes have been made in this RKSettingsModule */
	bool hasChanges ();
/** applies current settings in this RKSettingsModule. This will only be called, if hasChanges () is true */
	void applyChanges ();
/** saves current changes to the given KConfig
@param config probably always RKGlobals::rkApp ()->config. But passing this as an argument is both more flexible and saves #including files.*/
	void save (KConfig *config);

/** @returns the caption ("Workspace Browser") */
	QString caption ();

	static void saveSettings (KConfig *config);
	static void loadSettings (KConfig *config);

/** whether hidden variables should be shown in RObjectListViews by default */
	static bool showHiddenVars () { return (options & ShowHiddenVars); };
/** whether the type field should be shown in RObjectListViews by default */
	static bool showTypeField () { return (options & ShowTypeField); };
/** whether the class field should be shown in RObjectListViews by default */
	static bool showClassField () { return (options & ShowClassField); };
/** whether the label field should be shown in RObjectListViews by default */
	static bool showLabelField () { return (options & ShowLabelField); };
	
	enum ObjectBrowserOptions { ShowHiddenVars=1, ShowTypeField=2, ShowClassField=4, ShowLabelField=8 };
public slots:
/** called when a checkbox has been changed. Signals change to RKSettings dialog to enable apply button */
	void boxChanged (int);
private:
	QCheckBox *show_hidden_vars_box;
	QCheckBox *show_type_field_box;
	QCheckBox *show_class_field_box;
	QCheckBox *show_label_field_box;

	static int options;
};

#endif
