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
#include <kdeversion.h>
#ifdef RKWARD_USE_KNS3
#	include <knewstuff3/downloaddialog.h>
#else
#	include <knewstuff2/engine.h>
#endif
#include <ktar.h>
#include <kzip.h>
#include <kio/deletejob.h>

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
#include "rksettingsmodulegeneral.h"

#include "../debug.h"

// static members
QStringList RKSettingsModulePlugins::plugin_maps;
QStringList RKSettingsModulePlugins::known_plugin_maps;
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

	connect (button_group, SIGNAL (buttonClicked (int)), this, SLOT (settingChanged ()));
	main_vbox->addWidget (button_box);


	main_vbox->addSpacing (2*RKGlobals::spacingHint ());


	QGroupBox *code_frame = new QGroupBox (i18n ("R syntax display (in dialogs)"), this);
	group_layout = new QVBoxLayout (code_frame);

	show_code_box = new QCheckBox (i18n ("Code shown by default"), code_frame);
	show_code_box->setChecked (show_code);
	connect (show_code_box, SIGNAL (stateChanged (int)), this, SLOT (settingChanged ()));
	group_layout->addWidget (show_code_box);

	KHBox *code_size_hbox = new KHBox (code_frame);
	new QLabel (i18n ("Default height of code display (pixels)"), code_size_hbox);
	code_size_box = new RKSpinBox (code_size_hbox);
	code_size_box->setIntMode (20, 5000, code_size);
	connect (code_size_box, SIGNAL (valueChanged (int)), this, SLOT (settingChanged ()));
	group_layout->addWidget (code_size_hbox);

	main_vbox->addWidget (code_frame);


	main_vbox->addSpacing (2*RKGlobals::spacingHint ());


	map_choser = new MultiStringSelector (i18n ("Select .pluginmap file(s)"), this);
	map_choser->setValues (plugin_maps);
	connect (map_choser, SIGNAL (getNewStrings (QStringList*)), this, SLOT (browseRequest (QStringList*)));
	connect (map_choser, SIGNAL (listChanged ()), this, SLOT (settingChanged ()));
	main_vbox->addWidget (map_choser);

	main_vbox->addSpacing (2*RKGlobals::spacingHint ());
	button = new QPushButton (i18n ("Install or uninstall add-on plugin packs"), this);
	main_vbox->addWidget (button);
	connect (button, SIGNAL (clicked()), this, SLOT (downloadPlugins()));

	main_vbox->addStretch ();
}

RKSettingsModulePlugins::~RKSettingsModulePlugins() {
	RK_TRACE (SETTINGS);
}

void RKSettingsModulePlugins::settingChanged () {
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

	fixPluginMapLists ();
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
	cg.writeEntry ("All known plugin maps", known_plugin_maps);
	cg.writeEntry ("Interface Preferences", static_cast<int> (interface_pref));
	cg.writeEntry ("Code display default", show_code);
	cg.writeEntry ("Code display size", code_size);
}

void RKSettingsModulePlugins::loadSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group ("Plugin Settings");
	plugin_maps = cg.readEntry ("Plugin Maps", QStringList ());
	known_plugin_maps = cg.readEntry ("All known plugin maps", QStringList ());
	if (plugin_maps.isEmpty ()) {
		plugin_maps.append (RKCommonFunctions::getRKWardDataDir () + "/all.pluginmap");
	}
	fixPluginMapLists ();

	interface_pref = static_cast<PluginPrefs> (cg.readEntry ("Interface Preferences", static_cast<int> (PreferRecommended)));
	show_code = cg.readEntry ("Code display default", false);
	code_size = cg.readEntry ("Code display size", 250);
	if (RKSettingsModuleGeneral::storedConfigVersion () <= RKSettingsModuleGeneral::RKWardConfig_Pre0_5_7) {
		if (code_size == 40) code_size = 250;	// previous default untouched.
	}
}

// static
void RKSettingsModulePlugins::registerPluginMaps (const QStringList &maps, bool force_add, bool force_reload) {
	RK_TRACE (SETTINGS);

	QStringList added;
	foreach (const QString &map, maps) {
		if (map.isEmpty ()) continue;
		if (known_plugin_maps.contains (map)) {
			if (!force_add) continue;
		} else {
			known_plugin_maps.append (map);
		}

		if (plugin_maps.contains (map)) continue;
		plugin_maps.append (map);
		added.append (map);
	}

	if (!added.isEmpty ()) {
		KMessageBox::informationList (RKWardMainWindow::getMain (), i18n ("New RKWard plugin packs (listed below) have been found, and have been activated, automatically. To de-activate selected plugin packs, use Settings->Configure RKWard->Plugins."), added, i18n ("New plugins found"), "new_plugins_found");
		force_reload = true;
	}

	if (force_reload) RKWardMainWindow::getMain ()->initPlugins();
}

void RKSettingsModulePlugins::fixPluginMapLists () {
	RK_TRACE (SETTINGS);

	for (int i = 0; i < plugin_maps.size (); ++i) {
		QFileInfo info (plugin_maps[i]);
		if (!info.isReadable ()) {
			known_plugin_maps.removeAll (plugin_maps[i]);
			plugin_maps.removeAt (i);
			--i;
		}
	}

	foreach (const QString &map, plugin_maps) {
		if (!known_plugin_maps.contains (map)) known_plugin_maps.append (map);
	}
}

void RKSettingsModulePlugins::downloadPlugins () {
	RK_TRACE (SETTINGS);

	QStringList oldmaps = plugin_maps;

#ifdef RKWARD_USE_KNS3
	KNS3::DownloadDialog dialog ("rkward.knsrc", 0);
	dialog.exec ();
	KNS3::Entry::List list = dialog.changedEntries ();
#else
	KNS::Engine engine (0);
	if (!engine.init ("rkward.knsrc")) return;
	KNS::Entry::List list = engine.downloadDialogModal (this);
#endif

	for (int i = 0; i < list.size (); ++i) {
#ifdef RKWARD_USE_KNS3
		QStringList installed_files = list[i].installedFiles ();
		QStringList uninstalled_files = list[i].uninstalledFiles ();
#else
		QStringList installed_files = list[i]->installedFiles ();
		QStringList uninstalled_files = list[i]->uninstalledFiles ();
#endif
		foreach (const QString inst, installed_files) {
			installPluginPack (inst);
		}
		foreach (const QString inst, uninstalled_files) {
			uninstallPluginPack (inst);
		}
	}

	// new pluginmaps were already added in installPluginPack. Now let's check, whether there any to be removed:
	fixPluginMapLists ();

	if (plugin_maps != oldmaps) {
		map_choser->setValues (plugin_maps);
		change ();
	}
}

void RKSettingsModulePlugins::installPluginPack (const QString &archive_file) {
	RK_TRACE (SETTINGS);

	QString basename = baseNameOfPluginPack (archive_file);
	if (basename.isEmpty ()) return;

	// remove any old versions of the same plugin. Unfortunately, KNewStuff does not clean up when installing updates.
	QFileInfo archive_file_info (archive_file);
	QFileInfo base_file_info (basename);
	QDir base_dir = base_file_info.absoluteDir ();
	QString base_filename = base_file_info.fileName ();
	QStringList old_versions = base_dir.entryList (QDir::Files).filter (QRegExp ("^" + base_filename + "(-.*)?\\.(tar\\.gz|\\zip)$"));
	foreach (const QString old_version, old_versions) {
		if (old_version != archive_file_info.fileName ()) QFile::remove (base_dir.absoluteFilePath (old_version));
	}
	// finally, remove the previous unpacked installation, if any, to make sure we have a clean install
	if (QDir().exists (basename)) KIO::del (KUrl::fromLocalFile (basename))->exec ();

	KArchive* archive;
	if (archive_file.endsWith (".zip", Qt::CaseInsensitive)) {
		archive = new KZip (archive_file);
	} else {
		archive = new KTar (archive_file);
	}
	if (!archive->open (QIODevice::ReadOnly)) {
#warning TODO: show error message
		RK_ASSERT (false);
		return;
	}
	archive->directory ()->copyTo (basename, true);
	delete (archive);

	QStringList installed_maps = findPluginMapsRecursive (basename);
	foreach (const QString map, installed_maps) {
		if (!plugin_maps.contains (map)) plugin_maps.append (map);
	}
}

void RKSettingsModulePlugins::uninstallPluginPack (const QString &archive_file) {
	RK_TRACE (SETTINGS);

	QString basename = baseNameOfPluginPack (archive_file);
	if (basename.isEmpty ()) return;

	// Well, calling exec is ugly, but so much simpler than handling this asynchronously...
	KIO::del (KUrl::fromLocalFile (basename))->exec ();
}

QString RKSettingsModulePlugins::baseNameOfPluginPack (const QString &archive_file) {
	RK_TRACE (SETTINGS);

	QFileInfo file_info (archive_file);
	QDir dir = file_info.absoluteDir ();
	QString name = file_info.fileName ();

	// strip file-type ending
	if (name.endsWith (".tar.gz", Qt::CaseInsensitive)) {
		name = name.left (name.length () - 7);
	} else if (name.endsWith (".zip", Qt::CaseInsensitive)) {
		name = name.left (name.length () - 4);
	} else {
		return QString ();
	}

	// strip version (if any)
	int where = name.indexOf ("-", 1);	// must have at least one char of name before version string
	if (where > 0) name = name.left (where);

	return dir.absoluteFilePath (name);
}

QStringList RKSettingsModulePlugins::findPluginMapsRecursive (const QString &basedir) {
	RK_TRACE (SETTINGS);

	QDir dir (basedir);
	QStringList maps = dir.entryList (QDir::Files).filter (QRegExp (".*\\.pluginmap$"));
	QStringList ret;
	foreach (const QString &map, maps) ret.append (dir.absoluteFilePath (map));

	QStringList subdirs = dir.entryList (QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
	foreach (const QString subdir, subdirs) {
#if QT_VERSION >= 0x040500
		ret.append (findPluginMapsRecursive (dir.absoluteFilePath (subdir)));
#else
		QStringList subs = findPluginMapsRecursive (dir.absoluteFilePath (subdir));
		foreach (const QString sub, subs) ret.append (sub);
#endif
	}

	return ret;
}

#include "rksettingsmoduleplugins.moc"
