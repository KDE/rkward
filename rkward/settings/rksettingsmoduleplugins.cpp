/***************************************************************************
                          rksettingsmoduleplugins  -  description
                             -------------------
    begin                : Wed Jul 28 2004
    copyright            : (C) 2004, 2006, 2007, 2010 by Thomas Friedrichsmeier
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
#include "rksettingsmoduleplugins.h"

#include <klocale.h>
#include <kconfig.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <khbox.h>
#include <knewstuff2/engine.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <QVBoxLayout>
#include <QPushButton>

#include "../rkward.h"
#include "../rkglobals.h"
#include "../misc/multistringselector.h"
#include "../misc/rkcommonfunctions.h"
#include "../misc/rkspinbox.h"

#include "../debug.h"

// static members
QStringList RKSettingsModulePlugins::plugin_maps;
RKSettingsModulePlugins::PluginPrefs RKSettingsModulePlugins::interface_pref;
bool RKSettingsModulePlugins::show_code;
int RKSettingsModulePlugins::code_size;

RKSettingsModulePlugins::RKSettingsModulePlugins (RKSettings *gui, QWidget *parent) : RKSettingsModule (gui, parent) {
	RK_TRACE (SETTINGS);

	QVBoxLayout *main_vbox = new QVBoxLayout (this);
	
	main_vbox->addSpacing (2*RKGlobals::spacingHint ());
	
	QLabel *label = new QLabel (i18n ("Some plugins are available with both, a wizard-like interface and a traditional dialog interface. If both are available, which mode of presentation do you prefer?"), this);
	label->setWordWrap (true);
	main_vbox->addWidget (label);


	QGroupBox* button_box = new QGroupBox (this);
	QVBoxLayout* group_layout = new QVBoxLayout (button_box);
	button_group = new QButtonGroup (button_box);

	QAbstractButton* button;
	button = new QRadioButton (i18n ("Always prefer dialogs"), button_box);
	group_layout->addWidget (button);
	button_group->addButton (button, PreferDialog);
	button = new QRadioButton (i18n ("Prefer recommended interface"), button_box);
	group_layout->addWidget (button);
	button_group->addButton (button, PreferRecommended);
	button = new QRadioButton (i18n ("Always prefer wizards"), button_box);
	group_layout->addWidget (button);
	button_group->addButton (button, PreferWizard);
	if ((button = button_group->button (interface_pref))) button->setChecked (true);

	connect (button_group, SIGNAL (buttonClicked (int)), this, SLOT (settingChanged (int)));
	main_vbox->addWidget (button_box);


	main_vbox->addSpacing (2*RKGlobals::spacingHint ());


	QGroupBox *code_frame = new QGroupBox (i18n ("R syntax display (in dialogs)"), this);
	group_layout = new QVBoxLayout (code_frame);

	show_code_box = new QCheckBox (i18n ("Code shown by default"), code_frame);
	show_code_box->setChecked (show_code);
	connect (show_code_box, SIGNAL (stateChanged (int)), this, SLOT (settingChanged (int)));
	group_layout->addWidget (show_code_box);

	KHBox *code_size_hbox = new KHBox (code_frame);
	new QLabel (i18n ("Default height of code display (pixels)"), code_size_hbox);
	code_size_box = new RKSpinBox (code_size_hbox);
	code_size_box->setIntMode (20, 5000, code_size);
	connect (code_size_box, SIGNAL (valueChanged (int)), this, SLOT (settingChanged (int)));
	group_layout->addWidget (code_size_hbox);

	main_vbox->addWidget (code_frame);


	main_vbox->addSpacing (2*RKGlobals::spacingHint ());


	map_choser = new MultiStringSelector (i18n ("Select .pluginmap file(s)"), this);
	map_choser->setValues (plugin_maps);
	connect (map_choser, SIGNAL (getNewStrings (QStringList*)), this, SLOT (browseRequest (QStringList*)));
	connect (map_choser, SIGNAL (listChanged ()), this, SLOT (pathsChanged ()));
	main_vbox->addWidget (map_choser);

#warning REMEMBER TO CLEAN UP
	main_vbox->addSpacing (2*RKGlobals::spacingHint ());
	button = new QPushButton ("Push me, Meik", this);
	main_vbox->addWidget (button);
	connect (button, SIGNAL (clicked()), this, SLOT (downloadPlugins()));

	main_vbox->addStretch ();
}

RKSettingsModulePlugins::~RKSettingsModulePlugins() {
	RK_TRACE (SETTINGS);
}

void RKSettingsModulePlugins::pathsChanged () {
	RK_TRACE (SETTINGS);
	change ();
}

void RKSettingsModulePlugins::settingChanged (int) {
	RK_TRACE (SETTINGS);
	change ();
}

void RKSettingsModulePlugins::browseRequest (QStringList* strings) {
	RK_TRACE (SETTINGS);

	(*strings) = KFileDialog::getOpenFileNames (RKCommonFunctions::getRKWardDataDir (), "*.pluginmap", this, i18n ("Select .pluginmap-file"));
}

QString RKSettingsModulePlugins::caption () {
	RK_TRACE (SETTINGS);
	return (i18n ("Plugins"));
}

bool RKSettingsModulePlugins::hasChanges () {
	RK_TRACE (SETTINGS);
	return changed;
}

void RKSettingsModulePlugins::applyChanges () {
	RK_TRACE (SETTINGS);

	plugin_maps = map_choser->getValues ();
	interface_pref = static_cast<PluginPrefs> (button_group->checkedId ());
	show_code = show_code_box->isChecked ();
	code_size = code_size_box->intValue ();

	RKWardMainWindow::getMain ()->initPlugins();
}

void RKSettingsModulePlugins::save (KConfig *config) {
	RK_TRACE (SETTINGS);
	saveSettings (config);
}

void RKSettingsModulePlugins::saveSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group ("Plugin Settings");
	cg.writeEntry ("Plugin Maps", plugin_maps);
	cg.writeEntry ("Interface Preferences", static_cast<int> (interface_pref));
	cg.writeEntry ("Code display default", show_code);
	cg.writeEntry ("Code display size", code_size);
}

void RKSettingsModulePlugins::loadSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group ("Plugin Settings");
	plugin_maps = cg.readEntry ("Plugin Maps", QStringList ());
	if (plugin_maps.isEmpty ()) {
		plugin_maps.append (RKCommonFunctions::getRKWardDataDir () + "/all.pluginmap");
	}

	interface_pref = static_cast<PluginPrefs> (cg.readEntry ("Interface Preferences", static_cast<int> (PreferRecommended)));
	show_code = cg.readEntry ("Code display default", false);
	code_size = cg.readEntry ("Code display size", 40);
}

void RKSettingsModulePlugins::downloadPlugins () {
	RK_TRACE (SETTINGS);

	KNS::Engine engine (0);
	if (engine.init ("rkward.knsrc")) {
		engine.downloadDialogModal (this);
	}
}


#include "rksettingsmoduleplugins.moc"
