/***************************************************************************
                          rksettingsmoduleplugins  -  description
                             -------------------
    begin                : Wed Jul 28 2004
    copyright            : (C) 2004-2013 by Thomas Friedrichsmeier
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
#include "../misc/xmlhelper.h"
#include "../plugin/rkcomponentmap.h"
#include "rksettingsmodulegeneral.h"

#include "../debug.h"

// static members
QList<RKSettingsModulePlugins::PluginMapStoredInfo> RKSettingsModulePlugins::known_plugin_maps;
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


	map_choser = new RKMultiStringSelectorV2 (i18n ("Select .pluginmap file(s)"), this);
	map_model = new RKSettingsModulePluginsModel (this);
	map_model->init (known_plugin_maps);
	map_choser->setModel (map_model, 1);
	connect (map_choser, SIGNAL (insertNewStrings(int)), map_model, SLOT (insertNewStrings(int)));
	connect (map_choser, SIGNAL (swapRows(int,int)), map_model, SLOT (swapRows(int,int)));
	connect (map_choser, SIGNAL (listChanged ()), this, SLOT (settingChanged ()));
	main_vbox->addWidget (map_choser);
}

RKSettingsModulePlugins::~RKSettingsModulePlugins() {
	RK_TRACE (SETTINGS);
}

void RKSettingsModulePlugins::settingChanged () {
	RK_TRACE (SETTINGS);
	change ();
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

	known_plugin_maps = map_model->pluginMaps ();
	interface_pref = static_cast<PluginPrefs> (button_group->checkedId ());
	show_code = show_code_box->isChecked ();
	code_size = code_size_box->intValue ();

	fixPluginMapLists ();
	RKWardMainWindow::getMain ()->initPlugins();
	map_choser->setModel (0);	// we don't want any extra change notification for this
	map_model->init (known_plugin_maps);
	map_choser->setModel (map_model, 1);
}

void RKSettingsModulePlugins::save (KConfig *config) {
	RK_TRACE (SETTINGS);
	saveSettings (config);
}

void RKSettingsModulePlugins::saveSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group ("Plugin Settings");
	cg.deleteGroup ("Known Plugin maps");	// always start from scratch to remove cruft from pluginmaps
	KConfigGroup pmg = cg.group ("Known Plugin maps");
	QStringList all_known_maps;
	for (int i = 0; i < known_plugin_maps.size (); ++i) {
		const PluginMapStoredInfo &inf = known_plugin_maps[i];
		KConfigGroup ppmg = pmg.group (inf.filename);
		ppmg.writeEntry ("Active", inf.active);
		ppmg.writeEntry ("Broken", inf.broken_in_this_version);
		ppmg.writeEntry ("Quirky", inf.quirky_in_this_version);
		ppmg.writeEntry ("timestamp", inf.last_modified);
		all_known_maps.append (inf.filename);
	}
	// NOTE: The group list is always sorted alphabetically, which is why we need a separate list setting for saving info on order.
	cg.writeEntry ("All known plugin maps", all_known_maps);

	cg.writeEntry ("Interface Preferences", static_cast<int> (interface_pref));
	cg.writeEntry ("Code display default", show_code);
	cg.writeEntry ("Code display size", code_size);
}

void RKSettingsModulePlugins::loadSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group ("Plugin Settings");
	if (RKSettingsModuleGeneral::storedConfigVersion () < RKSettingsModuleGeneral::RKWardConfig_0_6_1) {
		QStringList plugin_maps = cg.readEntry ("Plugin Maps", QStringList ());
		QStringList kplugin_maps = cg.readEntry ("All known plugin maps", QStringList ());
		for (int i = 0; i < kplugin_maps.size (); ++i) {
			PluginMapStoredInfo inf (kplugin_maps[i]);
			inf.active = plugin_maps.contains (kplugin_maps[i]);
			// state info will be properly initialized in fixPluginMapLists()
			known_plugin_maps.append (inf);
		}
	} else {
		KConfigGroup pmg = cg.group ("Known Plugin maps");
		QStringList kplugin_maps = cg.readEntry ("All known plugin maps", QStringList ());
		for (int i = 0; i < kplugin_maps.size (); ++i) {
			KConfigGroup ppmg = pmg.group (kplugin_maps[i]);
			PluginMapStoredInfo inf (kplugin_maps[i]);
			inf.active = ppmg.readEntry ("Active", false);
			// Pluginmaps which are broken with one version of RKWard may be alright with other versions. So reset flags, if version has changed.
			inf.broken_in_this_version = ppmg.readEntry ("Broken", false) && !RKSettingsModuleGeneral::rkwardVersionChanged ();
			inf.quirky_in_this_version = ppmg.readEntry ("Quirky", false) && !RKSettingsModuleGeneral::rkwardVersionChanged ();
			inf.last_modified = ppmg.readEntry ("timestamp", QDateTime ());
			known_plugin_maps.append (inf);
		}
	}
	fixPluginMapLists ();	// removes any maps which don't exist any more

	interface_pref = static_cast<PluginPrefs> (cg.readEntry ("Interface Preferences", static_cast<int> (PreferRecommended)));
	show_code = cg.readEntry ("Code display default", false);
	code_size = cg.readEntry ("Code display size", 250);
	if (RKSettingsModuleGeneral::storedConfigVersion () <= RKSettingsModuleGeneral::RKWardConfig_Pre0_5_7) {
		if (code_size == 40) code_size = 250;	// previous default untouched.
	}
}

int findKnownPluginMap (const QString& filename, const RKSettingsModulePlugins::PluginMapList& haystack) {
	RK_TRACE (SETTINGS);

	int i;
	for (i = haystack.size () - 1; i >= 0; --i) {
		if (haystack[i].filename == filename) return i;
	}
	return i;
}

bool RKSettingsModulePlugins::markPluginMapAsBroken (const QString& map) {
	RK_TRACE (SETTINGS);

	int index = findKnownPluginMap (map, known_plugin_maps);
	if (index < 0) {
		RK_ASSERT (index >= 0);
		return false;
	}
	bool ret = !known_plugin_maps[index].broken_in_this_version;
	known_plugin_maps[index].broken_in_this_version = true;
	known_plugin_maps[index].active = false;
	return ret;
}

bool RKSettingsModulePlugins::markPluginMapAsQuirky (const QString& map) {
	RK_TRACE (SETTINGS);

	int index = findKnownPluginMap (map, known_plugin_maps);
	if (index < 0) {
		RK_ASSERT (index >= 0);
		return false;
	}
	bool ret = !known_plugin_maps[index].quirky_in_this_version;
	known_plugin_maps[index].quirky_in_this_version = true;
	return ret;
}

QStringList RKSettingsModulePlugins::pluginMaps () {
	RK_TRACE (SETTINGS);

	QStringList ret;
	for (int i = 0; i < known_plugin_maps.size (); ++i) {
		if (known_plugin_maps[i].active) ret.append (known_plugin_maps[i].filename);
	}
	return ret;
}

// static
void RKSettingsModulePlugins::registerPluginMaps (const QStringList &maps, bool force_add, bool force_reload) {
	RK_TRACE (SETTINGS);

	QStringList added;
	foreach (const QString &map, maps) {
		if (map.isEmpty ()) continue;
		int index = findKnownPluginMap (map, known_plugin_maps);
		if (index >= 0) {
			if (!force_add) continue;
		} else {	// not found
			PluginMapStoredInfo inf (map);
			known_plugin_maps.append (inf);
			index = known_plugin_maps.size () - 1;
		}

		if (!known_plugin_maps[index].active) {
			known_plugin_maps[index].active = true;
			added.append (map);
		}
	}

	if (force_reload || (!added.isEmpty ())) {
		RKWardMainWindow::getMain ()->initPlugins (added);
	}
}

void RKSettingsModulePlugins::fixPluginMapLists () {
	RK_TRACE (SETTINGS);

	QFileInfo default_pluginmap (RKCommonFunctions::getRKWardDataDir () + "/all.pluginmap");
	int default_pluginmap_index = -1;
	bool any_active_pluginmap = false;

	for (int i = 0; i < known_plugin_maps.size (); ++i) {
		PluginMapStoredInfo &inf = known_plugin_maps[i];
		QFileInfo info (inf.filename);
		if (!info.isReadable ()) {
			known_plugin_maps.removeAt (i);
			--i;
		} else {
			if (inf.active) any_active_pluginmap = true;
			if ((default_pluginmap_index < 0) && (info == default_pluginmap)) default_pluginmap_index = i;

			if (info.lastModified () != inf.last_modified) {
				inf.broken_in_this_version = false;
				inf.quirky_in_this_version = false;
				inf.last_modified = info.lastModified ();
			}
		}
	}

	// make sure the default plugin map is in the list (unless it is non-readable)
	if ((default_pluginmap_index < 0) && (default_pluginmap.isReadable ())) {
		PluginMapStoredInfo inf (default_pluginmap.absoluteFilePath ());
		known_plugin_maps.prepend (inf);
		default_pluginmap_index = 0;
	}

	// if no other pluginmap is active, activate the default map
	if (!any_active_pluginmap && (default_pluginmap_index >= 0)) {
		known_plugin_maps[default_pluginmap_index].active = true;
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
		// TODO: show error message
		RK_ASSERT (false);
		return;
	}
	archive->directory ()->copyTo (basename, true);
	delete (archive);

	QStringList installed_maps = findPluginMapsRecursive (basename);
	registerPluginMaps (installed_maps, false, false);
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

RKSettingsModulePluginsModel::RKSettingsModulePluginsModel (RKSettingsModulePlugins* parent) : QAbstractTableModel (parent) {
	RK_TRACE (SETTINGS);
}

RKSettingsModulePluginsModel::~RKSettingsModulePluginsModel() {
	RK_TRACE (SETTINGS);
	foreach (const PluginMapMetaInfo &inf, plugin_map_dynamic_info) {
		delete (inf.about);
	}
}

void RKSettingsModulePluginsModel::init (const RKSettingsModulePlugins::PluginMapList& known_plugin_maps) {
	RK_TRACE (SETTINGS);
	plugin_maps = known_plugin_maps;
	emit (reset ());
}

int RKSettingsModulePluginsModel::rowCount (const QModelIndex& parent) const {
	//RK_TRACE (SETTINGS);
	if (parent.isValid ()) return 0;
	return plugin_maps.size ();
}

#define COLUMN_CHECKED 0
#define COLUMN_FILENAME 1
#define COLUMN_STATUS 2
#define COLUMN_COUNT 3

int RKSettingsModulePluginsModel::columnCount (const QModelIndex& parent) const {
	// RK_TRACE (SETTINGS);
	if (parent.isValid ()) return 0;
	return COLUMN_COUNT;
}

QVariant RKSettingsModulePluginsModel::data (const QModelIndex& index, int role) const {
	// RK_TRACE (SETTINGS);
	if (!index.isValid ()) return QVariant ();
	int col = index.column ();

	const RKSettingsModulePlugins::PluginMapStoredInfo &inf = plugin_maps[index.row ()];

	if (role == Qt::BackgroundRole) {
		if (inf.broken_in_this_version) return Qt::red;
		if (inf.quirky_in_this_version) return Qt::yellow;
		return (QVariant ());
	} else if (role == Qt::ToolTipRole) {
		const PluginMapMetaInfo &meta = const_cast<RKSettingsModulePluginsModel*> (this)->getPluginMapMetaInfo (inf.filename);
		QString desc = meta.about->toHtml ();
		if (!meta.dependencies.isEmpty ()) {
			desc.append ("<b>" + i18n ("Dependencies") + "</b>");
			desc.append (RKComponentDependency::depsToHtml (meta.dependencies));
		}
		return desc;
	}

	if (col == COLUMN_CHECKED) {
		if (role == Qt::CheckStateRole) {
			return (inf.active ? Qt::Checked : Qt::Unchecked);
		}
	} else if (col == COLUMN_FILENAME) {
		if (role == Qt::DisplayRole) return inf.filename;
	} else if (col == COLUMN_STATUS) {
		if (role == Qt::DisplayRole) {
			if (inf.broken_in_this_version) return i18n ("Broken");
			QString status;
			if (RKComponentMap::getMap ()->isPluginMapLoaded (inf.filename)) status = i18n ("Loaded");
			if (inf.quirky_in_this_version) {
				if (!status.isEmpty ()) status.append (", ");
				status.append (i18n ("Quirky"));
			}
			return status;
		}
	}

	return QVariant ();
}

Qt::ItemFlags RKSettingsModulePluginsModel::flags (const QModelIndex& index) const {
	// RK_TRACE (SETTINGS);
	Qt::ItemFlags flags = QAbstractItemModel::flags (index);
	if (index.isValid () && (index.column () == COLUMN_CHECKED)) flags |= Qt::ItemIsUserCheckable | Qt::ItemIsEditable;
	return flags;
}

QVariant RKSettingsModulePluginsModel::headerData (int section, Qt::Orientation orientation, int role) const {
	// RK_TRACE (SETTINGS);
	if ((role == Qt::DisplayRole) && (orientation == Qt::Horizontal)) {
		if (section == COLUMN_CHECKED) return i18n ("Active");
		if (section == COLUMN_FILENAME) return i18n ("Filename");
		if (section == COLUMN_STATUS) return i18n ("Status");
		RK_ASSERT (false);
	}
	return QVariant ();
}

bool RKSettingsModulePluginsModel::setData (const QModelIndex& index, const QVariant& value, int role) {
	RK_TRACE (SETTINGS);

	if (role == Qt::CheckStateRole) {
		if (index.isValid () && (index.column () == COLUMN_CHECKED)) {
			plugin_maps[index.row ()].active = value.toBool ();
			emit (dataChanged (index, index));
			return true;
		}
	}

	return QAbstractItemModel::setData (index, value, role);
}

void RKSettingsModulePluginsModel::insertNewStrings (int above_row) {
	RK_TRACE (SETTINGS);

	QStringList files = KFileDialog::getOpenFileNames (RKCommonFunctions::getRKWardDataDir (), "*.pluginmap", static_cast<QWidget*> (QObject::parent ()), i18n ("Select .pluginmap-file"));

	// already known files are activated, but not added
	for (int i = files.size () -1; i >= 0; --i) {
		int pos = findKnownPluginMap (files[i], plugin_maps);
		if (pos >= 0) {
			if (!plugin_maps[pos].active) {
				plugin_maps[pos].active = true;
				emit (dataChanged (index (pos, 0), index (pos, COLUMN_COUNT - 1)));
			}
			files.removeAt (i);
		} 
	}

	beginInsertRows (QModelIndex (), above_row, files.size ());
	for (int i = files.size () - 1; i >= 0; --i) {
		RKSettingsModulePlugins::PluginMapStoredInfo inf (files[i]);
		inf.active = true;
		plugin_maps.insert (above_row, inf);
	}
	endInsertRows ();
}

void RKSettingsModulePluginsModel::swapRows (int rowa, int rowb) {
	RK_TRACE (SETTINGS);

	RK_ASSERT ((rowa >= 0) && (rowa < rowCount ()) && (rowb >= 0) && (rowb < rowCount ()));
	RKSettingsModulePlugins::PluginMapStoredInfo inf = plugin_maps[rowa];
	plugin_maps[rowa] = plugin_maps[rowb];
	plugin_maps[rowb] = inf;
}

bool RKSettingsModulePluginsModel::removeRows (int row, int count, const QModelIndex& parent) {
	RK_TRACE (SETTINGS);
	RK_ASSERT (!parent.isValid ());

	if ((row < 0) || (count < 1) || (row + count > rowCount () - 1)) return false;
	for (int i = row + count - 1; i >= row; --i) {
		plugin_maps.removeAt (i);
	}
	return true;
}

const RKSettingsModulePluginsModel::PluginMapMetaInfo& RKSettingsModulePluginsModel::getPluginMapMetaInfo (const QString& pluginmapfile) {
	RK_TRACE (SETTINGS);
	if (!plugin_map_dynamic_info.contains (pluginmapfile)) {
		// TODO
		PluginMapMetaInfo inf;
		XMLHelper *xml = XMLHelper::getStaticHelper ();
		QDomElement doc_elem = xml->openXMLFile (pluginmapfile, DL_WARNING);
		inf.about = new RKComponentAboutData (xml->getChildElement (doc_elem, "about", DL_INFO));
		inf.dependencies = RKComponentDependency::parseDependencies (xml->getChildElement (doc_elem, "dependencies", DL_INFO));
		plugin_map_dynamic_info.insert (pluginmapfile, inf);
	}

	return (plugin_map_dynamic_info[pluginmapfile]);
}

#include "rksettingsmoduleplugins.moc"
