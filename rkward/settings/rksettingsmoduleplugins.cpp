/*
rksettingsmoduleplugins - This file is part of RKWard (https://rkward.kde.org). Created: Wed Jul 28 2004
SPDX-FileCopyrightText: 2004-2020 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rksettingsmoduleplugins.h"

#include <KLocalizedString>
#include <kmessagebox.h>
#include <KSharedConfig>
#include <KConfigGroup>

#include <qlayout.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QIcon>

#include "../rkward.h"
#include "../misc/rkstyle.h"
#include "../misc/multistringselector.h"
#include "../misc/rkcommonfunctions.h"
#include "../misc/rkspinbox.h"
#include "../misc/xmlhelper.h"
#include "../plugin/rkcomponentmap.h"
#include "../dialogs/rkloadlibsdialog.h"
#include "rksettingsmodulegeneral.h"

#include "../debug.h"

// static members
RKSettingsModulePlugins::UniquePluginMapList RKSettingsModulePlugins::known_plugin_maps;
RKConfigValue<RKSettingsModulePlugins::PluginPrefs, int> RKSettingsModulePlugins::interface_pref {"Interface Preferences", RKSettingsModulePlugins::PreferRecommended};
RKConfigValue<bool> RKSettingsModulePlugins::show_code {"Code display default", false};
RKConfigValue<int> RKSettingsModulePlugins::code_size {"Code display size", 250};
RKConfigValue<int> RKSettingsModulePlugins::side_preview_width {"Other preview size", 250};

RKSettingsModulePlugins::RKSettingsModulePlugins (RKSettings *gui, QWidget *parent) : RKSettingsModule (gui, parent) {
	RK_TRACE (SETTINGS);

	QVBoxLayout *main_vbox = new QVBoxLayout (this);
	
	main_vbox->addSpacing (2*RKStyle::spacingHint ());
	
	main_vbox->addWidget (RKCommonFunctions::wordWrappedLabel (i18n ("Some plugins are available with both, a wizard-like interface and a traditional dialog interface. If both are available, which mode of presentation do you prefer?")));


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

	connect (button_group, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &RKSettingsModulePlugins::settingChanged);
	main_vbox->addWidget (button_box);

	main_vbox->addSpacing (2*RKStyle::spacingHint ());

	QPushButton *pluginmap_config_button = new QPushButton (i18n ("Configure Active Plugins"), this);
	connect (pluginmap_config_button, &QPushButton::clicked, this, &RKSettingsModulePlugins::configurePluginmaps);
	main_vbox->addWidget (pluginmap_config_button);

	main_vbox->addStretch ();
}

RKSettingsModulePlugins::~RKSettingsModulePlugins() {
	RK_TRACE (SETTINGS);
}

void RKSettingsModulePlugins::settingChanged () {
	RK_TRACE (SETTINGS);
	change ();
}

QString RKSettingsModulePlugins::caption() const {
	RK_TRACE(SETTINGS);
	return(i18n("RKWard Plugins"));
}

QIcon RKSettingsModulePlugins::icon() const {
	RK_TRACE(SETTINGS);
	return QIcon::fromTheme("plugins");
}

void RKSettingsModulePlugins::applyChanges () {
	RK_TRACE (SETTINGS);

	interface_pref = static_cast<PluginPrefs> (button_group->checkedId ());
}

RKSettingsModulePlugins::UniquePluginMapList RKSettingsModulePlugins::setPluginMaps(const UniquePluginMapList &new_list) {
	RK_TRACE (SETTINGS);

	known_plugin_maps = new_list;
	fixPluginMapLists ();
	RKWardMainWindow::getMain ()->initPlugins();
	return known_plugin_maps;
}

void RKSettingsModulePlugins::configurePluginmaps () {
	RK_TRACE (SETTINGS);

	RKLoadLibsDialog::showPluginmapConfig (this, commandChain ());
}

void savePluginMaps(KConfigGroup &cg, const RKSettingsModulePlugins::UniquePluginMapList &known_plugin_maps) {
	RK_TRACE(SETTINGS);

	cg.deleteGroup("Known Plugin maps");	// always start from scratch to remove cruft from pluginmaps
	KConfigGroup pmg = cg.group("Known Plugin maps");
	QStringList all_known_maps;
	for (int i = 0; i < known_plugin_maps.size(); ++i) {
		auto &variants = known_plugin_maps[i].maps;
		for (int j = 0; j < variants.size(); ++j) {
			const RKSettingsModulePlugins::PluginMapStoredInfo &inf = variants[j];
			KConfigGroup ppmg = pmg.group (inf.filename);
			ppmg.writeEntry("Active", known_plugin_maps[i].active);
			ppmg.writeEntry("State", (int) inf.state);
			ppmg.writeEntry("timestamp", inf.last_modified);
			ppmg.writeEntry("id", inf.id);
			ppmg.writeEntry("version", inf.version.toString());
			ppmg.writeEntry("priority", inf.priority);
			all_known_maps.append(inf.filename);
		}
	}
	// NOTE: The group list is always sorted alphabetically, which is why we need a separate list setting for saving info on order.
	cg.writeEntry("All known plugin maps", all_known_maps);
}

void RKSettingsModulePlugins::syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction a) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group("Plugin Settings");
	interface_pref.syncConfig(cg, a);
	show_code.syncConfig(cg, a);
	code_size.syncConfig(cg, a);
	side_preview_width.syncConfig(cg, a);

	if (a == RKConfigBase::SaveConfig) {
		savePluginMaps(cg, known_plugin_maps);
	} else {
		/* Known maps are stored at runtime as a nested list id -> variants, but stored in config as a plain list of variants. This is for historic reasons, but may be too much trouble to change. */
		KConfigGroup pmg = cg.group ("Known Plugin maps");
		QStringList kplugin_maps = cg.readEntry ("All known plugin maps", QStringList ());
		for (int i = 0; i < kplugin_maps.size (); ++i) {
			KConfigGroup ppmg = pmg.group (kplugin_maps[i]);	// unadjusted path on purpose!
			PluginMapStoredInfo inf (RKSettingsModuleGeneral::checkAdjustLoadedPath (kplugin_maps[i]));
			// Pluginmaps which are broken with one version of RKWard may be alright with other versions. So reset flags, if version has changed.
			if (RKSettingsModuleGeneral::rkwardVersionChanged()) inf.state = Working;
			else inf.state = (PluginMapState) ppmg.readEntry("State", (int) Working);
			inf.last_modified = ppmg.readEntry ("timestamp", QDateTime ());
			inf.id = ppmg.readEntry ("id");
			inf.version = ppmg.readEntry ("version");
			inf.priority = ppmg.readEntry ("priority", (int) PriorityMedium);
			addPluginMapToList(inf, ppmg.readEntry("Active", false));
		}
		if (RKSettingsModuleGeneral::rkwardVersionChanged () || RKSettingsModuleGeneral::installationMoved ()) {
			// if it is the first start this version or from a new path, scan the installation for new pluginmaps
			// Note that in the case of installationMoved(), checkAdjustLoadedPath() has already kicked in, above, but rescanning is still useful
			// e.g. if users have installed to a new location, because they had botched their previous installation
			registerDefaultPluginMaps(AutoActivateIfNew);
		}
		fixPluginMapLists ();	// removes any maps which don't exist any more
	}
}

bool RKSettingsModulePlugins::addPluginMapToList(const RKSettingsModulePlugins::PluginMapStoredInfo& inf, bool active) {
	RK_TRACE (SETTINGS);
	QString id = inf.id;
	QString filename = inf.filename;
	for (int i = 0; i < known_plugin_maps.size(); ++i) {
		UniquePluginMap &unique = known_plugin_maps[i];
		if (id == unique.id) {
			PluginMapList &variants = unique.maps;
			for (int j = 0; j < variants.size(); ++j) {
				if (variants[j].filename == filename) {
					variants[j] = inf;
					return false;
				}
			}
			variants.append(inf);
			unique.active = unique.active || active;
			return true;
		}
	}
	UniquePluginMap new_entry(id, active);
	new_entry.maps.append(inf);
	known_plugin_maps.append(new_entry);
	return true;
}

RKSettingsModulePlugins::PluginMapStoredInfo RKSettingsModulePlugins::UniquePluginMap::bestMap() const {
	RK_TRACE(SETTINGS);
	RK_ASSERT(!maps.isEmpty());

	PluginMapStoredInfo candidate = maps.first();
	for (int i = 1; i < maps.size(); ++i) {
		const PluginMapStoredInfo &other = maps[i];
		bool other_is_better = true;
		if (candidate.version > other.version) {
			other_is_better = false;
		} else if (candidate.version == other.version && candidate.last_modified > other.last_modified) {
			other_is_better = false;
		}
		if (!other_is_better) {
			candidate = other;
		}
	}
	return candidate;
}

void RKSettingsModulePlugins::registerDefaultPluginMaps(AddMode add_mode) {
	RK_TRACE (SETTINGS);

	QDir def_plugindir (RKCommonFunctions::getRKWardDataDir ());
	if (def_plugindir.dirName() == QStringLiteral ("rkwardinstall")) {
		// For running from build-dir: Work around bad design choice of installation layout
		def_plugindir.cd ("plugins");
	}
	QStringList def_pluginmaps = def_plugindir.entryList (QStringList ("*.pluginmap"));
	for (int i = 0; i < def_pluginmaps.size (); ++i) {
		def_pluginmaps[i] = def_plugindir.absoluteFilePath (def_pluginmaps[i]);
	}
	registerPluginMaps (def_pluginmaps, add_mode, false, add_mode == AutoActivateIfNew);
}

int findKnownPluginMap (const QString& filename, const RKSettingsModulePlugins::PluginMapList& haystack) {
	RK_TRACE (SETTINGS);

	int i;
	for (i = haystack.size () - 1; i >= 0; --i) {
		if (haystack[i].filename == filename) return i;
	}
	return i;
}

QString RKSettingsModulePlugins::findPluginMapById (const QString &id) {
	RK_TRACE (SETTINGS);

	for (int i = 0; i < known_plugin_maps.size (); ++i) {
		if (known_plugin_maps[i].id == id) return known_plugin_maps[i].bestMap().filename;
	}
	// for "rkward::" namespace, try a little harded:
	if (id.startsWith (QLatin1String ("rkward::"))) {
		QFileInfo info (RKCommonFunctions::getRKWardDataDir () + '/' + id.mid (8));
		if (info.isReadable ()) return info.absoluteFilePath ();
	}

	return QString ();
}

bool RKSettingsModulePlugins::markPluginMapState(const QString& map, PluginMapState state) {
	RK_TRACE (SETTINGS);

	for (int i = 0; i < known_plugin_maps.size(); ++i) {
		auto &variants = known_plugin_maps[i].maps;
		for (int j = 0; j < variants.size(); ++j) {
			if (variants[j].filename == map) {
				bool ret = variants[j].state != state;
				variants[j].state = state;
				if (state == Broken) known_plugin_maps[i].active = false;
				return ret;
			}
		}
	}
	RK_ASSERT(false);
	return false;
}

QStringList RKSettingsModulePlugins::pluginMaps () {
	RK_TRACE (SETTINGS);

	QStringList ret;
	for (int i = 0; i < known_plugin_maps.size (); ++i) {
		if (known_plugin_maps[i].active) ret.append (known_plugin_maps[i].bestMap().filename);
	}
	return ret;
}

// static
void RKSettingsModulePlugins::registerPluginMaps (const QStringList &maps, AddMode add_mode, bool force_reload, bool suppress_reload) {
	RK_TRACE (SETTINGS);

	QStringList added;
	foreach (const QString &map, maps) {
		if (map.isEmpty ()) continue;
		bool found = false;
		for (int i = 0; i < known_plugin_maps.size(); ++i) {
			const PluginMapList &variants = known_plugin_maps[i].maps;
			for (int j = 0; j < variants.size(); ++j) {
				const auto &inf = variants[j];
				if (inf.filename == map) {
					found = true;
					if (!known_plugin_maps[i].active) {
						if (add_mode == ForceActivate || (add_mode == AutoActivate && inf.priority >= PriorityMedium)) {
							known_plugin_maps[i].active = true;
							added.append(map);
						}
					}
					break;
				}
			}
			if (found) break;
		}

		if (!found) {
			PluginMapStoredInfo inf = parsePluginMapBasics(map);
			bool activate = (add_mode == ForceActivate) || (inf.priority >= PriorityMedium);
			addPluginMapToList(map, activate);
			if (activate) added.append(map);
		}
	}

	if (suppress_reload) return;
	if (force_reload || (!added.isEmpty ())) {
		RKWardMainWindow::getMain ()->initPlugins(added);
	}
}

void RKSettingsModulePlugins::fixPluginMapLists () {
	RK_TRACE(SETTINGS);

	for (int i = 0; i < known_plugin_maps.size(); ++i) {
		PluginMapList &variants = known_plugin_maps[i].maps;
		for (int j = 0; j < variants.size(); ++j) {
			PluginMapStoredInfo &inf = variants[j];
			QFileInfo info(inf.filename);
			if (!info.isReadable()) {
				variants.removeAt(j);
				--j;
				continue;
			}

			if (info.lastModified () != inf.last_modified || inf.version.isNull()) {
				inf = parsePluginMapBasics(inf.filename);
				inf.last_modified = info.lastModified();
				// TOOD: Handle the case that it might have changed id? Should be remedied on the next config load, anyway, however.
			}
		}
		if (variants.isEmpty()) {
			known_plugin_maps.removeAt(i);
			--i;
			continue;
		}
	}
}

RKSettingsModulePlugins::PluginMapStoredInfo RKSettingsModulePlugins::parsePluginMapBasics(const QString &filename) {
	RK_TRACE(SETTINGS);

	XMLHelper xml(filename);
	QDomElement de = xml.openXMLFile(DL_WARNING);
	PluginMapStoredInfo inf(filename);
	inf.id = RKPluginMapFile::parseId(de, xml);
	inf.priority = xml.getMultiChoiceAttribute(de, "priority", "hidden;low;medium;high", (int) PriorityMedium, DL_WARNING);
	auto about = xml.getChildElement(de, "about", DL_WARNING);
	inf.version = xml.getStringAttribute(about, "version", QString(), DL_WARNING);
	return inf;
}

QStringList RKSettingsModulePlugins::findPluginMapsRecursive (const QString &basedir) {
	RK_TRACE (SETTINGS);

	QDir dir (basedir);
	QStringList maps = dir.entryList (QDir::Files).filter (QRegExp (".*\\.pluginmap$"));
	QStringList ret;
	foreach (const QString &map, maps) ret.append (dir.absoluteFilePath (map));

	QStringList subdirs = dir.entryList (QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
	foreach (const QString& subdir, subdirs) {
		ret.append (findPluginMapsRecursive (dir.absoluteFilePath (subdir)));
	}

	return ret;
}

RKSettingsModulePluginsModel::RKSettingsModulePluginsModel (QObject* parent) : QAbstractTableModel (parent) {
	RK_TRACE (SETTINGS);
}

RKSettingsModulePluginsModel::~RKSettingsModulePluginsModel() {
	RK_TRACE (SETTINGS);
	foreach (const PluginMapMetaInfo &inf, plugin_map_dynamic_info) {
		delete (inf.about);
	}
}

void RKSettingsModulePluginsModel::init (const RKSettingsModulePlugins::UniquePluginMapList& known_plugin_maps) {
	RK_TRACE (SETTINGS);
	beginResetModel ();
	plugin_maps = known_plugin_maps;
	endResetModel ();
}

int RKSettingsModulePluginsModel::rowCount (const QModelIndex& parent) const {
	//RK_TRACE (SETTINGS);
	if (parent.isValid ()) return 0;
	return plugin_maps.size ();
}

#define COLUMN_CHECKED 0
#define COLUMN_TITLE 1
#define COLUMN_ID 2
#define COLUMN_STATUS 3
#define COLUMN_COUNT 4

int RKSettingsModulePluginsModel::columnCount (const QModelIndex& parent) const {
	// RK_TRACE (SETTINGS);
	if (parent.isValid ()) return 0;
	return COLUMN_COUNT;
}

QVariant RKSettingsModulePluginsModel::data (const QModelIndex& index, int role) const {
	// RK_TRACE (SETTINGS);
	if (!index.isValid ()) return QVariant ();
	int col = index.column ();

	const auto &unique = plugin_maps[index.row()];
	const auto &inf = unique.bestMap();  // TODO: efficiency

	if (role == Qt::BackgroundRole) {
		if (inf.state == RKSettingsModulePlugins::Broken) return QColor(Qt::red);
		if (inf.state == RKSettingsModulePlugins::Quirky) return QColor(Qt::yellow);
		return (QVariant ());
	} else if (role == Qt::ForegroundRole) {
		if (inf.priority < RKSettingsModulePlugins::PriorityLow) return QColor (Qt::gray);
	} else if (role == Qt::ToolTipRole) {
		const PluginMapMetaInfo &meta = const_cast<RKSettingsModulePluginsModel*> (this)->getPluginMapMetaInfo (inf.filename);
		QString desc = meta.about->toHtml ();
		if (!meta.dependencies.isEmpty ()) {
			desc.append ("<b>" + i18n ("Dependencies") + "</b>");
			desc.append (RKComponentDependency::depsToHtml (meta.dependencies));
		}
		desc.append ("<p>" + inf.filename + "</p>");
		return desc;
	}

	if (col == COLUMN_CHECKED) {
		if (role == Qt::CheckStateRole) {
			if (inf.priority < RKSettingsModulePlugins::PriorityLow) return QVariant ();
			return (unique.active ? Qt::Checked : Qt::Unchecked);
		}
	} else if (col == COLUMN_ID) {
		if (role == Qt::DisplayRole) {
			return inf.id;
		}
	} else if (col == COLUMN_TITLE) {
		if (role == Qt::DisplayRole) {
			const PluginMapMetaInfo &meta = const_cast<RKSettingsModulePluginsModel*> (this)->getPluginMapMetaInfo (inf.filename);
			return meta.about->name;
		}
	} else if (col == COLUMN_STATUS) {
		if (role == Qt::DisplayRole) {
			if (inf.state == RKSettingsModulePlugins::Broken) return i18n ("Broken");
			QString status;
			if (RKComponentMap::getMap ()->isPluginMapLoaded (inf.filename)) status = i18n ("Loaded");
			if (inf.state == RKSettingsModulePlugins::Quirky) {
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
	Qt::ItemFlags flags = QAbstractTableModel::flags (index);
	if (index.isValid () && (index.column () == COLUMN_CHECKED)) {
		if (plugin_maps[index.row()].bestMap().priority > RKSettingsModulePlugins::PriorityHidden) flags |= Qt::ItemIsUserCheckable | Qt::ItemIsEditable;
	}
	return flags;
}

QVariant RKSettingsModulePluginsModel::headerData (int section, Qt::Orientation orientation, int role) const {
	// RK_TRACE (SETTINGS);
	if ((role == Qt::DisplayRole) && (orientation == Qt::Horizontal)) {
		if (section == COLUMN_CHECKED) return i18n ("Active");
		if (section == COLUMN_ID) return i18n ("ID");
		if (section == COLUMN_TITLE) return i18n ("Title");
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
			emit dataChanged(index, index);
			return true;
		}
	}

	return QAbstractItemModel::setData (index, value, role);
}

void RKSettingsModulePluginsModel::insertNewStrings (int above_row) {
	RK_TRACE (SETTINGS);

	QStringList files = QFileDialog::getOpenFileNames (static_cast<QWidget*> (QObject::parent ()), i18n ("Select .pluginmap-file"), RKCommonFunctions::getRKWardDataDir (), "RKWard pluginmap files [*.pluginmap](*.pluginmap)");

	// already known files are activated, but not added
	for (int i = files.size () -1; i >= 0; --i) {
		int pos = findKnownPluginMap (files[i], plugin_maps);
		if (pos >= 0) {
			if (!plugin_maps[pos].active) {
				plugin_maps[pos].active = true;
				emit dataChanged(index(pos, 0), index(pos, COLUMN_COUNT - 1));
			}
			files.removeAt (i);
		}
	}

	beginInsertRows (QModelIndex (), above_row, files.size ());
	for (int i = files.size () - 1; i >= 0; --i) {
		auto inf = RKSettingsModulePlugins::parsePluginMapBasics(files[i]);
		RKSettingsModulePlugins::UniquePluginMap unique(files[i], true);
		unique.maps.append(inf);
		plugin_maps.insert(above_row, unique);
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

	if ((row < 0) || (count < 1) || (row + count > rowCount ())) return false;
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
		XMLHelper xml (pluginmapfile);
		QDomElement doc_elem = xml.openXMLFile (DL_WARNING);
		inf.about = new RKComponentAboutData (xml.getChildElement (doc_elem, "about", DL_INFO), xml);
		inf.dependencies = RKComponentDependency::parseDependencies (xml.getChildElement (doc_elem, "dependencies", DL_INFO), xml);
		plugin_map_dynamic_info.insert (pluginmapfile, inf);
	}

	return (plugin_map_dynamic_info[pluginmapfile]);
}

