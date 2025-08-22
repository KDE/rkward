/*
rksettingsmoduleplugins - This file is part of RKWard (https://rkward.kde.org). Created: Wed Jul 28 2004
SPDX-FileCopyrightText: 2004-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rksettingsmoduleplugins.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <kmessagebox.h>

#include <QFileDialog>
#include <QIcon>
#include <QPushButton>
#include <QVBoxLayout>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>

#include "../dialogs/rkloadlibsdialog.h"
#include "../misc/multistringselector.h"
#include "../misc/rkcommonfunctions.h"
#include "../misc/rkradiogroup.h"
#include "../misc/rkspinbox.h"
#include "../misc/rkstyle.h"
#include "../misc/xmlhelper.h"
#include "../plugin/rkcomponentmap.h"
#include "../rkward.h"
#include "../windows/rkworkplace.h"
#include "rksettings.h"
#include "rksettingsmodulegeneral.h"

#include "../debug.h"

// static members
RKSettingsModulePlugins::RKPluginMapList RKSettingsModulePlugins::known_plugin_maps;
RKConfigValue<RKSettingsModulePlugins::PluginPrefs, int> RKSettingsModulePlugins::interface_pref{"Interface Preferences", RKSettingsModulePlugins::PreferRecommended};
RKConfigValue<bool> RKSettingsModulePlugins::show_code{"Code display default", false};
RKConfigValue<int> RKSettingsModulePlugins::code_size{"Code display size", 250};
RKConfigValue<int> RKSettingsModulePlugins::side_preview_width{"Other preview size", 250};

#include <QLabel>

class RKSettingsPagePlugins : public RKSettingsModuleWidget {
  public:
	RKSettingsPagePlugins(QWidget *parent, RKSettingsModule *parent_module) : RKSettingsModuleWidget(parent, parent_module, RKSettingsModulePlugins::page_id, RKSettingsModulePlugins::addons_superpage_id) {
		RK_TRACE(SETTINGS);

		setWindowTitle(i18n("RKWard Plugins"));
		setWindowIcon(QIcon::fromTheme(QStringLiteral("plugins")));

		QVBoxLayout *main_vbox = new QVBoxLayout(this);
		main_vbox->addSpacing(2 * RKStyle::spacingHint());
		main_vbox->addWidget(RKCommonFunctions::wordWrappedLabel(i18n("Some plugins are available with both, a wizard-like interface and a traditional dialog interface. If both are available, which mode of presentation do you prefer?")));

		auto button_box = new RKRadioGroup();
		button_group = button_box->group();
		button_box->addButton(i18n("Always prefer dialogs"), RKSettingsModulePlugins::PreferDialog);
		button_box->addButton(i18n("Prefer recommended interface"), RKSettingsModulePlugins::PreferRecommended);
		button_box->addButton(i18n("Always prefer wizards"), RKSettingsModulePlugins::PreferWizard);
		button_box->setButtonChecked(RKSettingsModulePlugins::interface_pref, true);
		connect(button_group, &QButtonGroup::idClicked, this, &RKSettingsPagePlugins::change);
		main_vbox->addWidget(button_box);

		main_vbox->addSpacing(2 * RKStyle::spacingHint());

		QPushButton *pluginmap_config_button = new QPushButton(i18n("Configure Active Plugins"), this);
		connect(pluginmap_config_button, &QPushButton::clicked, this, [this]() { RKLoadLibsDialog::showPluginmapConfig(this, parentModule()->commandChain()); });
		main_vbox->addWidget(pluginmap_config_button);

		main_vbox->addStretch();
	}
	void applyChanges() override {
		RK_TRACE(SETTINGS);

		RKSettingsModulePlugins::interface_pref = static_cast<RKSettingsModulePlugins::PluginPrefs>(button_group->checkedId());
	}

  private:
	QButtonGroup *button_group;
};

class RKSettingsHeaderPage : public RKSettingsModuleWidget {
  public:
	RKSettingsHeaderPage(QWidget *parent, RKSettingsModule *parent_module) : RKSettingsModuleWidget(parent, parent_module, RKSettingsModulePlugins::addons_superpage_id) {
		setWindowTitle(i18n("Add-ons"));
		auto layout = new QVBoxLayout(this);
		QLabel *l = new QLabel(i18n("<h1>Add-ons</h1><p>RKWard add-ons come in a variety of forms, each with their own configuration options:</p><h2>R packages</h2><p><a href=\"rkward://settings/rpackages\">Add-ons to the R language itself</a>. These are usually downloaded from \"CRAN\". Some of these add-on packages may additionally contain RKWard plugins.</p><h2>RKWard plugins</h2><p><a href=\"rkward://settings/plugins\">Graphical dialogs to R functionality</a>. These plugins are usually pre-installed with RKWard, or with an R package. However, they can be activated/deactivated to help keep the menus manageable. Note that it is relatively easy to <a href=\"https://api.kde.org/doc/rkwardplugins/\">create your own custom dialogs as plugins</a>!</p><h2>Kate plugins</h2><p><a href=\"rkward://settings/kateplugins\">Plugins developed for Kate / KTextEditor</a>. These provide shared functionality that is useful in the context of text editing and IDE applications. These plugins are usually found pre-installed on your system. You can configure to load the plugins that are useful to your own workflow.</p>"));
		l->setWordWrap(true);
		connect(l, &QLabel::linkActivated, [=](const QString &url) { RKWorkplace::mainWorkplace()->openAnyUrl(QUrl(url)); });
		layout->addWidget(l);
		layout->addStretch();
	}
	void applyChanges() override {};
};

RKSettingsModulePlugins::RKSettingsModulePlugins(QObject *parent) : RKSettingsModule(parent) {
	RK_TRACE(SETTINGS);
}

RKSettingsModulePlugins::~RKSettingsModulePlugins() {
	RK_TRACE(SETTINGS);
}

void RKSettingsModulePlugins::createPages(RKSettings *parent) {
	parent->addSettingsPage(new RKSettingsHeaderPage(parent, this));
	parent->addSettingsPage(new RKSettingsPagePlugins(parent, this));
}

RKSettingsModulePlugins::RKPluginMapList RKSettingsModulePlugins::setPluginMaps(const RKPluginMapList &new_list) {
	RK_TRACE(SETTINGS);

	known_plugin_maps = new_list;
	known_plugin_maps.removeObsoleteMaps();
	RKWardMainWindow::getMain()->initPlugins();
	return known_plugin_maps;
}

void RKSettingsModulePlugins::RKPluginMapList::saveToConfig(KConfigGroup &cg) {
	RK_TRACE(SETTINGS);

	cg.deleteGroup(QStringLiteral("Known Plugin maps")); // always start from scratch to remove cruft from pluginmaps
	KConfigGroup pmg = cg.group(QStringLiteral("Known Plugin maps"));
	QStringList kplugin_maps;
	for (auto it = all_maps.constBegin(); it != all_maps.constEnd(); ++it) {
		const auto &maps = it.value().list;
		for (int j = 0; j < maps.size(); ++j) {
			const RKSettingsModulePlugins::PluginMapStoredInfo &inf = maps[j];
			KConfigGroup ppmg = pmg.group(inf.filename);
			kplugin_maps.append(inf.filename);
			ppmg.writeEntry("Active", it.value().active);
			ppmg.writeEntry("State", (int)inf.state);
			ppmg.writeEntry("timestamp", inf.last_modified);
			ppmg.writeEntry("id", inf.id);
			ppmg.writeEntry("version", inf.version.toString());
			ppmg.writeEntry("priority", inf.priority);
		}
	}
	// NOTE: The group list is always sorted alphabetically, which is why we need to save a separate list setting for saving info on order.
	cg.writeEntry("Pluginmap id order", ordered_ids);
	// I'd like to store this implicitly as cg.groupList(), alas that doesn't seem to work (KF5 5.68.0)
	cg.writeEntry("All known plugin maps", kplugin_maps);
}

void RKSettingsModulePlugins::RKPluginMapList::readFromConfig(KConfigGroup &cg) {
	RK_TRACE(SETTINGS);
	ordered_ids.clear();
	all_maps.clear();

	/* Known maps are stored at runtime as a nested list id -> variants, but stored in config as a plain list of variants. This is for historic reasons, but may be too much trouble to change. */
	KConfigGroup pmg = cg.group(QStringLiteral("Known Plugin maps"));
	QStringList kplugin_maps = cg.readEntry("All known plugin maps", QStringList());
	for (int i = 0; i < kplugin_maps.size(); ++i) {
		KConfigGroup ppmg = pmg.group(kplugin_maps[i]); // unadjusted path on purpose!
		PluginMapStoredInfo inf(RKSettingsModuleGeneral::checkAdjustLoadedPath(kplugin_maps[i]));
		// Pluginmaps which are broken with one version of RKWard may be alright with other versions. So reset flags, if version has changed.
		if (RKSettingsModuleGeneral::rkwardVersionChanged()) inf.state = Working;
		else inf.state = (PluginMapState)ppmg.readEntry("State", (int)Working);
		inf.last_modified = ppmg.readEntry("timestamp", QDateTime());
		inf.id = ppmg.readEntry("id");
		inf.version = RKParsedVersion(ppmg.readEntry("version"));
		inf.priority = ppmg.readEntry("priority", (int)PriorityMedium);
		addMap(inf, ppmg.readEntry("Active", false) ? ForceActivate : DoNotActivate);
	}
	if (RKSettingsModuleGeneral::rkwardVersionChanged() || RKSettingsModuleGeneral::installationMoved()) {
		// if it is the first start this version or from a new path, scan the installation for new pluginmaps
		// Note that in the case of installationMoved(), checkAdjustLoadedPath() has already kicked in, above, but rescanning is still useful
		// e.g. if users have installed to a new location, because they had botched their previous installation
		registerDefaultPluginMaps(AutoActivateIfNew);
	}
	removeObsoleteMaps();
	QStringList nordered_ids = cg.readEntry("Pluginmap id order", ordered_ids);
	// make sure order has all relevant ids, and only those, before applying
	for (int i = nordered_ids.size() - 1; i >= 0; --i) {
		if (!ordered_ids.contains(nordered_ids[i])) nordered_ids.removeAt(i);
	}
	for (int i = 0; i < ordered_ids.size(); ++i) {
		if (!nordered_ids.contains(ordered_ids[i])) nordered_ids.append(ordered_ids[i]);
	}
	ordered_ids = nordered_ids;
}

void RKSettingsModulePlugins::syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction a) {
	RK_TRACE(SETTINGS);

	KConfigGroup cg = config->group(QStringLiteral("Plugin Settings"));
	interface_pref.syncConfig(cg, a);
	show_code.syncConfig(cg, a);
	code_size.syncConfig(cg, a);
	side_preview_width.syncConfig(cg, a);

	if (a == RKConfigBase::SaveConfig) {
		known_plugin_maps.saveToConfig(cg);
	} else {
		known_plugin_maps.readFromConfig(cg);
	}
}

bool RKSettingsModulePlugins::PluginMapStoredInfo::betterThan(const RKSettingsModulePlugins::PluginMapStoredInfo &other) const {
	if (version > other.version) return true;
	if (version == other.version && last_modified > other.last_modified) return true;
	return false;
}

bool RKSettingsModulePlugins::RKPluginMapList::addMap(const PluginMapStoredInfo &inf, AddMode add_mode) {
	RK_TRACE(SETTINGS);

	QFileInfo info(inf.filename);
	if (!info.isReadable()) return false;
	if (info.lastModified() != inf.last_modified || inf.version.isNull()) {
		auto inf2 = parsePluginMapBasics(inf.filename);
		inf2.last_modified = info.lastModified();
		if (inf2.version.isNull()) inf2.version = RKParsedVersion(QStringLiteral("0.0.0.1")); // prevent infinite recursion
		return addMap(inf2, add_mode);
	}

	bool activate = (add_mode == ForceActivate || (add_mode == AutoActivate && inf.priority >= PriorityMedium));
	QString id = inf.id;
	QString filename = inf.filename;
	bool known = false;
	PluginMapInfoList &sub_list = all_maps[id].list;
	activate |= all_maps[id].active;
	for (int i = 0; i < sub_list.size(); ++i) {
		if (sub_list[i].filename == filename) {
			sub_list[i] = inf;
			known = true;
			break;
		}
	}
	if (!known) {
		activate = activate || (add_mode == AutoActivateIfNew && inf.priority >= PriorityMedium);
		if (sub_list.isEmpty()) ordered_ids.append(id);
		sub_list.append(inf);
		std::sort(sub_list.begin(), sub_list.end(), [](const PluginMapStoredInfo &a, const PluginMapStoredInfo &b) { return a.betterThan(b); });
	}
	if (activate) {
		setIdActive(id, activate);
	}

	return !known;
}

void RKSettingsModulePlugins::RKPluginMapList::removeObsoleteMaps() {
	RK_TRACE(SETTINGS);

	QSet<QString> ids_to_remove;
	for (auto it = all_maps.begin(); it != all_maps.end(); ++it) {
		auto &sublist = it.value();
		for (int i = 0; i < sublist.list.size(); ++i) {
			QString filename = sublist.list[i].filename;
			if (!QFile::exists(filename)) {
				sublist.list.removeAt(i);
				--i;
				break;
			}
		}
		if (sublist.list.isEmpty()) ids_to_remove.insert(it.key());
	}
	for (auto it = ids_to_remove.constBegin(); it != ids_to_remove.constEnd(); ++it) {
		forgetId(*it);
	}
}

bool RKSettingsModulePlugins::RKPluginMapList::setMapfileState(const QString &mapfile, PluginMapState state) {
	RK_TRACE(SETTINGS);
	for (auto it = all_maps.begin(); it != all_maps.end(); ++it) {
		auto &sublist = it.value().list;
		for (int i = 0; i < sublist.size(); ++i) {
			if (mapfile == sublist[i].filename) {
				bool ret = (sublist[i].state != state);
				sublist[i].state = state;
				if (state == Broken) it.value().active = false;
				return ret;
			}
		}
	}
	return false;
}

void RKSettingsModulePlugins::RKPluginMapList::forgetId(const QString &id) {
	RK_TRACE(SETTINGS);
	all_maps.remove(id);
	ordered_ids.removeAll(id);
}

void RKSettingsModulePlugins::RKPluginMapList::setIdActive(const QString &id, bool active) {
	RK_TRACE(SETTINGS);
	all_maps[id].active = active;
	if (all_maps[id].list.isEmpty()) {
		RK_ASSERT(false);
		forgetId(id);
	}
}

QString RKSettingsModulePlugins::RKPluginMapList::mapFileForId(const QString &id) const {
	RK_TRACE(SETTINGS);
	auto val = all_maps.value(id);
	if (val.list.isEmpty()) return QString();
	return val.list.first().filename;
}

QList<RKSettingsModulePlugins::PluginMapStoredInfo> RKSettingsModulePlugins::RKPluginMapList::allUniqueMaps() {
	RK_TRACE(SETTINGS);
	QList<RKSettingsModulePlugins::PluginMapStoredInfo> ret;
	for (int i = 0; i < ordered_ids.size(); ++i) {
		auto val = all_maps[ordered_ids[i]];
		if (!val.list.isEmpty()) ret.append(val.list.first());
	}
	return ret;
}

QStringList RKSettingsModulePlugins::RKPluginMapList::activeMapFiles() {
	RK_TRACE(SETTINGS);
	QStringList ret;
	for (int i = 0; i < ordered_ids.size(); ++i) {
		auto val = all_maps[ordered_ids[i]];
		if (val.active && !val.list.isEmpty()) ret.append(val.list.first().filename);
	}
	return ret;
}

void RKSettingsModulePlugins::registerDefaultPluginMaps(AddMode add_mode) {
	RK_TRACE(SETTINGS);

	QDir def_plugindir(RKCommonFunctions::getRKWardDataDir());
	if (def_plugindir.dirName() == "rkwardinstall"_L1) {
		// For running from build-dir: Work around bad design choice of installation layout
		def_plugindir.cd(u"plugins"_s);
	}
	QStringList def_pluginmaps = def_plugindir.entryList(QStringList(u"*.pluginmap"_s));
	for (int i = 0; i < def_pluginmaps.size(); ++i) {
		def_pluginmaps[i] = def_plugindir.absoluteFilePath(def_pluginmaps[i]);
	}
	registerPluginMaps(def_pluginmaps, add_mode, false, add_mode == AutoActivateIfNew);
}

QString RKSettingsModulePlugins::findPluginMapById(const QString &id) {
	RK_TRACE(SETTINGS);

	QString ret = known_plugin_maps.mapFileForId(id);
	if (!ret.isNull()) return ret;

	// for "rkward::" namespace, try a little harded:
	if (id.startsWith("rkward::"_L1)) {
		QFileInfo info(RKCommonFunctions::getRKWardDataDir() + u'/' + id.mid(8));
		if (info.isReadable()) return info.absoluteFilePath();
	}

	return QString();
}

bool RKSettingsModulePlugins::markPluginMapState(const QString &map, PluginMapState state) {
	RK_TRACE(SETTINGS);

	return known_plugin_maps.setMapfileState(map, state);
}

QStringList RKSettingsModulePlugins::pluginMaps() {
	RK_TRACE(SETTINGS);

	return known_plugin_maps.activeMapFiles();
}

// static
void RKSettingsModulePlugins::registerPluginMaps(const QStringList &maps, AddMode add_mode, bool force_reload, bool suppress_reload) {
	RK_TRACE(SETTINGS);

	QStringList added;
	for (const QString &map : maps) {
		if (map.isEmpty()) continue;
		if (known_plugin_maps.addMap(PluginMapStoredInfo(map), add_mode)) {
			added.append(map);
		}
	}

	if (suppress_reload) return;
	if (force_reload || (!added.isEmpty())) {
		RKWardMainWindow::getMain()->initPlugins(added); // NOTE: All maps get initialized, "added" is just for user notification of what got added
	}
}

RKSettingsModulePlugins::PluginMapStoredInfo RKSettingsModulePlugins::parsePluginMapBasics(const QString &filename) {
	RK_TRACE(SETTINGS);

	XMLHelper xml(filename);
	QDomElement de = xml.openXMLFile(DL_WARNING);
	PluginMapStoredInfo inf(filename);
	inf.id = RKPluginMapFile::parseId(de, xml);
	inf.priority = xml.getMultiChoiceAttribute(de, u"priority"_s, u"hidden;low;medium;high"_s, (int)PriorityMedium, DL_WARNING);
	auto about = xml.getChildElement(de, u"about"_s, DL_WARNING);
	inf.version = RKParsedVersion(xml.getStringAttribute(about, u"version"_s, QString(), DL_WARNING));
	return inf;
}

QStringList RKSettingsModulePlugins::findPluginMapsRecursive(const QString &basedir) {
	RK_TRACE(SETTINGS);

	QDir dir(basedir);
	static const QRegularExpression pluginmapfile_ext(u".*\\.pluginmap$"_s);
	const QStringList maps = dir.entryList(QDir::Files).filter(pluginmapfile_ext);
	QStringList ret;
	for (const QString &map : maps) {
		ret.append(dir.absoluteFilePath(map));
	}

	const QStringList subdirs = dir.entryList(QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
	for (const QString &subdir : subdirs) {
		ret.append(findPluginMapsRecursive(dir.absoluteFilePath(subdir)));
	}

	return ret;
}

RKSettingsModulePluginsModel::RKSettingsModulePluginsModel(QObject *parent) : QAbstractTableModel(parent) {
	RK_TRACE(SETTINGS);
}

RKSettingsModulePluginsModel::~RKSettingsModulePluginsModel() {
	RK_TRACE(SETTINGS);
	for (const PluginMapMetaInfo &inf : std::as_const(plugin_map_dynamic_info)) {
		delete (inf.about);
	}
}

void RKSettingsModulePluginsModel::init(const RKSettingsModulePlugins::RKPluginMapList &known_plugin_maps) {
	RK_TRACE(SETTINGS);
	beginResetModel();
	plugin_maps = known_plugin_maps;
	endResetModel();
}

int RKSettingsModulePluginsModel::rowCount(const QModelIndex &parent) const {
	// RK_TRACE (SETTINGS);
	if (parent.isValid()) return 0;
	return plugin_maps.ordered_ids.size();
}

#define COLUMN_CHECKED 0
#define COLUMN_TITLE 1
#define COLUMN_ID 2
#define COLUMN_STATUS 3
#define COLUMN_COUNT 4

int RKSettingsModulePluginsModel::columnCount(const QModelIndex &parent) const {
	// RK_TRACE (SETTINGS);
	if (parent.isValid()) return 0;
	return COLUMN_COUNT;
}

QVariant RKSettingsModulePluginsModel::data(const QModelIndex &index, int role) const {
	// RK_TRACE (SETTINGS);
	if (!index.isValid()) return QVariant();
	int col = index.column();

	const auto id = plugin_maps.ordered_ids.value(index.row());
	const auto &handle = plugin_maps.all_maps.value(id);
	if (handle.list.isEmpty()) return QVariant();
	const auto &inf = handle.list.first();

	if (role == Qt::BackgroundRole) {
		if (inf.state == RKSettingsModulePlugins::Broken) return QColor(Qt::red);
		if (inf.state == RKSettingsModulePlugins::Quirky) return QColor(Qt::yellow);
		return (QVariant());
	} else if (role == Qt::ForegroundRole) {
		if (inf.priority < RKSettingsModulePlugins::PriorityLow) return QColor(Qt::gray);
	} else if (role == Qt::ToolTipRole) {
		const PluginMapMetaInfo &meta = const_cast<RKSettingsModulePluginsModel *>(this)->getPluginMapMetaInfo(inf.filename);
		QString desc = meta.about->toHtml();
		if (!meta.dependencies.isEmpty()) {
			desc.append(u"<b>"_s + i18n("Dependencies") + u"</b>"_s);
			desc.append(RKComponentDependency::depsToHtml(meta.dependencies));
		}
		desc.append(u"<p>"_s + inf.filename + u"</p>"_s);
		return desc;
	}

	if (col == COLUMN_CHECKED) {
		if (role == Qt::CheckStateRole) {
			if (inf.priority < RKSettingsModulePlugins::PriorityLow) return QVariant();
			return (handle.active ? Qt::Checked : Qt::Unchecked);
		}
	} else if (col == COLUMN_ID) {
		if (role == Qt::DisplayRole) {
			return inf.id;
		}
	} else if (col == COLUMN_TITLE) {
		if (role == Qt::DisplayRole) {
			const PluginMapMetaInfo &meta = const_cast<RKSettingsModulePluginsModel *>(this)->getPluginMapMetaInfo(inf.filename);
			return meta.about->name;
		}
	} else if (col == COLUMN_STATUS) {
		if (role == Qt::DisplayRole) {
			if (inf.state == RKSettingsModulePlugins::Broken) return i18n("Broken");
			QString status;
			if (RKComponentMap::getMap()->isPluginMapLoaded(inf.filename)) status = i18n("Loaded");
			if (inf.state == RKSettingsModulePlugins::Quirky) {
				if (!status.isEmpty()) status.append(u", "_s);
				status.append(i18n("Quirky"));
			}
			return status;
		}
	}

	return QVariant();
}

Qt::ItemFlags RKSettingsModulePluginsModel::flags(const QModelIndex &index) const {
	// RK_TRACE (SETTINGS);
	Qt::ItemFlags flags = QAbstractTableModel::flags(index);
	if (index.isValid() && (index.column() == COLUMN_CHECKED)) {
		const auto id = plugin_maps.ordered_ids.value(index.row());
		const auto &handle = plugin_maps.all_maps.value(id);
		if (handle.list.isEmpty()) return flags;
		if (handle.list.first().priority > RKSettingsModulePlugins::PriorityHidden) flags |= Qt::ItemIsUserCheckable | Qt::ItemIsEditable;
	}
	return flags;
}

QVariant RKSettingsModulePluginsModel::headerData(int section, Qt::Orientation orientation, int role) const {
	// RK_TRACE (SETTINGS);
	if ((role == Qt::DisplayRole) && (orientation == Qt::Horizontal)) {
		if (section == COLUMN_CHECKED) return i18n("Active");
		if (section == COLUMN_ID) return i18n("ID");
		if (section == COLUMN_TITLE) return i18n("Title");
		if (section == COLUMN_STATUS) return i18n("Status");
		RK_ASSERT(false);
	}
	return QVariant();
}

bool RKSettingsModulePluginsModel::setData(const QModelIndex &index, const QVariant &value, int role) {
	RK_TRACE(SETTINGS);

	if (role == Qt::CheckStateRole) {
		if (index.isValid() && (index.column() == COLUMN_CHECKED)) {
			const auto id = plugin_maps.ordered_ids.value(index.row());
			if (!plugin_maps.all_maps.contains(id)) return false;
			auto &handle = plugin_maps.all_maps[id];
			handle.active = value.toBool();
			Q_EMIT dataChanged(index, index);
			return true;
		}
	}

	return QAbstractItemModel::setData(index, value, role);
}

void RKSettingsModulePluginsModel::insertNewStrings(int above_row) {
	RK_TRACE(SETTINGS);

	QStringList files = QFileDialog::getOpenFileNames(static_cast<QWidget *>(QObject::parent()), i18n("Select .pluginmap-file"), RKCommonFunctions::getRKWardDataDir(), QStringLiteral("RKWard pluginmap files [*.pluginmap](*.pluginmap)"));

	beginResetModel();
	// not bothering with proper rowsInserted signals, this does not need to be efficient, anyway.
	for (int i = files.size() - 1; i >= 0; --i) {
		auto inf = RKSettingsModulePlugins::parsePluginMapBasics(files[i]);
		plugin_maps.addMap(inf, RKSettingsModulePlugins::ForceActivate);
		int index = plugin_maps.ordered_ids.indexOf(inf.id);
		if (index < 0 || index > above_row) {
			plugin_maps.ordered_ids.removeAll(inf.id);
			plugin_maps.ordered_ids.insert(above_row, inf.id);
		}
	}
	endResetModel();
}

void RKSettingsModulePluginsModel::swapRows(int rowa, int rowb) {
	RK_TRACE(SETTINGS);

	RK_ASSERT((rowa >= 0) && (rowa < rowCount()) && (rowb >= 0) && (rowb < rowCount()));
	QString id = plugin_maps.ordered_ids[rowa];
	plugin_maps.ordered_ids[rowa] = plugin_maps.ordered_ids[rowb];
	plugin_maps.ordered_ids[rowb] = id;
}

bool RKSettingsModulePluginsModel::removeRows(int row, int count, const QModelIndex &parent) {
	RK_TRACE(SETTINGS);
	RK_ASSERT(!parent.isValid());

	if ((row < 0) || (count < 1) || (row + count > rowCount())) return false;
	for (int i = row + count - 1; i >= row; --i) {
		plugin_maps.forgetId(plugin_maps.ordered_ids.value(i));
	}
	return true;
}

const RKSettingsModulePluginsModel::PluginMapMetaInfo &RKSettingsModulePluginsModel::getPluginMapMetaInfo(const QString &pluginmapfile) {
	RK_TRACE(SETTINGS);
	if (!plugin_map_dynamic_info.contains(pluginmapfile)) {
		// TODO
		PluginMapMetaInfo inf;
		XMLHelper xml(pluginmapfile);
		QDomElement doc_elem = xml.openXMLFile(DL_WARNING);
		inf.about = new RKComponentAboutData(xml.getChildElement(doc_elem, QStringLiteral("about"), DL_INFO), xml);
		inf.dependencies = RKComponentDependency::parseDependencies(xml.getChildElement(doc_elem, QStringLiteral("dependencies"), DL_INFO), xml);
		plugin_map_dynamic_info.insert(pluginmapfile, inf);
	}

	return (plugin_map_dynamic_info[pluginmapfile]);
}
