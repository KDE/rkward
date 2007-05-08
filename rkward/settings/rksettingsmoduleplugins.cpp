/***************************************************************************
                          rksettingsmoduleplugins  -  description
                             -------------------
    begin                : Wed Jul 28 2004
    copyright            : (C) 2004, 2006, 2007 by Thomas Friedrichsmeier
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

#include <qlayout.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qvgroupbox.h>
#include <qcheckbox.h>
#include <qhbox.h>

#include "../rkward.h"
#include "../rkglobals.h"
#include "../misc/multistringselector.h"
#include "../misc/rkcommonfunctions.h"
#include "../misc/rkspinbox.h"

// static members
QStringList RKSettingsModulePlugins::plugin_maps;
RKSettingsModulePlugins::PluginPrefs RKSettingsModulePlugins::interface_pref;
bool RKSettingsModulePlugins::show_code;
int RKSettingsModulePlugins::code_size;

RKSettingsModulePlugins::RKSettingsModulePlugins (RKSettings *gui, QWidget *parent) : RKSettingsModule (gui, parent) {
	QVBoxLayout *main_vbox = new QVBoxLayout (this, RKGlobals::marginHint ());
	
	main_vbox->addSpacing (2*RKGlobals::spacingHint ());
	
	QLabel *label = new QLabel (i18n ("Some plugins are available with both, a wizard-like interface and a traditional dialog interface. If both are available, which mode of presentation do you prefer?"), this);
	label->setAlignment (Qt::AlignAuto | Qt::AlignVCenter | Qt::ExpandTabs | Qt::WordBreak);
	main_vbox->addWidget (label);
	
	button_group = new QButtonGroup (this);
	button_group->setColumnLayout (0, Qt::Vertical);
	button_group->layout()->setSpacing (6);
	button_group->layout()->setMargin (11);
	QVBoxLayout *group_layout = new QVBoxLayout(button_group->layout());
	group_layout->addWidget (new QRadioButton (i18n ("Always prefer dialogs"), button_group));
	group_layout->addWidget (new QRadioButton (i18n ("Prefer recommended interface"), button_group));
	group_layout->addWidget (new QRadioButton (i18n ("Always prefer wizards"), button_group));
	button_group->setButton (static_cast<int> (interface_pref));
	connect (button_group, SIGNAL (clicked (int)), this, SLOT (settingChanged (int)));
	main_vbox->addWidget (button_group);
	
	main_vbox->addSpacing (2*RKGlobals::spacingHint ());

	QVGroupBox *code_frame = new QVGroupBox (i18n ("R syntax display (in dialogs)"), this);
	show_code_box = new QCheckBox (i18n ("Code shown by default"), code_frame);
	show_code_box->setChecked (show_code);
	connect (show_code_box, SIGNAL (stateChanged (int)), this, SLOT (settingChanged (int)));

	QHBox *code_size_hbox = new QHBox (code_frame);
	new QLabel (i18n ("Default height of code display (pixels)"), code_size_hbox);
	code_size_box = new RKSpinBox (code_size_hbox);
	code_size_box->setIntMode (20, 5000, code_size);
	connect (code_size_box, SIGNAL (valueChanged (int)), this, SLOT (settingChanged (int)));
	main_vbox->addWidget (code_frame);
	
	main_vbox->addSpacing (2*RKGlobals::spacingHint ());

	map_choser = new MultiStringSelector (i18n ("Select .pluginmap file(s)"), this);
	map_choser->setValues (plugin_maps);
	connect (map_choser, SIGNAL (getNewStrings (QStringList*)), this, SLOT (browseRequest (QStringList*)));
	connect (map_choser, SIGNAL (listChanged ()), this, SLOT (pathsChanged ()));
	main_vbox->addWidget (map_choser);

	main_vbox->addStretch ();
}

RKSettingsModulePlugins::~RKSettingsModulePlugins() {
}

void RKSettingsModulePlugins::pathsChanged () {
	change ();
}

void RKSettingsModulePlugins::settingChanged (int) {
	change ();
}

void RKSettingsModulePlugins::browseRequest (QStringList* strings) {
	(*strings) = KFileDialog::getOpenFileNames (RKCommonFunctions::getRKWardDataDir (), "*.pluginmap", this, i18n ("Select .pluginmap-file"));
}

QString RKSettingsModulePlugins::caption () {
	return (i18n ("Plugins"));
}

bool RKSettingsModulePlugins::hasChanges () {
	return changed;
}

void RKSettingsModulePlugins::applyChanges () {
	plugin_maps = map_choser->getValues ();
#if QT_VERSION < 0x030200
	interface_pref = static_cast<PluginPrefs> (button_group->id (button_group->selected ()));
#else
	interface_pref = static_cast<PluginPrefs> (button_group->selectedId ());
#endif
	show_code = show_code_box->isChecked ();
	code_size = code_size_box->value ();

	RKWardMainWindow::getMain ()->initPlugins();
}

void RKSettingsModulePlugins::save (KConfig *config) {
	saveSettings (config);
}

void RKSettingsModulePlugins::saveSettings (KConfig *config) {
	config->setGroup ("Plugin Settings");
	config->writeEntry ("Plugin Maps", plugin_maps);
	config->writeEntry ("Interface Preferences", static_cast<int> (interface_pref));
	config->writeEntry ("Code display default", show_code);
	config->writeEntry ("Code display size", code_size);
}

void RKSettingsModulePlugins::loadSettings (KConfig *config) {
	config->setGroup ("Plugin Settings");
	plugin_maps = config->readListEntry ("Plugin Maps");
	if (!plugin_maps.count ()) {
		plugin_maps.append (RKCommonFunctions::getRKWardDataDir () + "/all.pluginmap");
	}
// TODO: this code is only needed for transition from rkward 0.3.4 to rkward 0.3.5. Remove some version later!
// BEGIN
	bool fix=false;
	for (QStringList::const_iterator it = plugin_maps.constBegin (); it != plugin_maps.constEnd (); ++it) {
		if ((*it).contains ("standard_plugins.pluginmap")) {
			fix = (KMessageBox::questionYesNo (0, i18n ("You appear to have an old configuration for the plugin-paths. The default configuration was changed between rkward 0.3.4 and rkward 0.3.5. Should the configuration be set to the new default (recommended)?"), i18n ("Configuration change"), KStdGuiItem::yes (), KStdGuiItem::no (), "pluginmap_upgrade") == KMessageBox::Yes);
		}
	}
	if (fix) {
		plugin_maps.clear ();
		plugin_maps.append (RKCommonFunctions::getRKWardDataDir () + "/all.pluginmap");
	}
// END

	interface_pref = static_cast<PluginPrefs> (config->readNumEntry ("Interface Preferences", static_cast<int> (PreferRecommended)));
	show_code = config->readBoolEntry ("Code display default", true);
	code_size = config->readNumEntry ("Code display size", 40);
}

#include "rksettingsmoduleplugins.moc"
