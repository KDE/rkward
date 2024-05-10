/*
rksettingsmoduleplugins - This file is part of the RKWard project. Created: Wed Jul 28 2004
SPDX-FileCopyrightText: 2004-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKSETTINGSMODULEPLUGINS_H
#define RKSETTINGSMODULEPLUGINS_H

#include "rksettingsmodule.h"
#include "../plugin/rkcomponentmeta.h"

#include <qstringlist.h>
#include <QDateTime>
#include <QAbstractTableModel>

class RKMultiStringSelectorV2;
class RKSettingsModulePluginsModel;
class QButtonGroup;
class RKSpinBox;

/** The settings-module for plugin specific settings
@author Thomas Friedrichsmeier
*/
class RKSettingsModulePlugins : public RKSettingsModule {
	Q_OBJECT
public:
	RKSettingsModulePlugins (RKSettings *gui, QWidget *parent);
	~RKSettingsModulePlugins ();

	void applyChanges () override;

	enum PluginPrefs { PreferDialog=0, PreferRecommended=1, PreferWizard=2 };

	void save(KConfig *config) override { syncConfig(config, RKConfigBase::SaveConfig); };
	static void syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction a);
	static void validateSettingsInteractive (QList<RKSetupWizardItem*>*) {};

	QString caption() const override;
	QIcon icon() const override;

	/** @returns a list of active plugin maps */
	static QStringList pluginMaps ();
	static PluginPrefs getInterfacePreference () { return interface_pref; };
	static bool showCodeByDefault () { return show_code; };
	static void setShowCodeByDefault (bool shown) { show_code = shown; };
	static int defaultCodeHeight () { return code_size; };
	static void setDefaultCodeHeight (int new_height) { code_size = new_height; }
	static int defaultSidePreviewWidth () { return side_preview_width; };
	static void setDefaultSidePreviewWidth (int new_width) { side_preview_width = new_width; }
	enum AddMode {
		DoNotActivate,
		ForceActivate,
		AutoActivate,
		AutoActivateIfNew
	};
	/** register a list of available plugin-maps (which may or may not already be known). New maps are activated, automatically.
	 * @param maps Plugin maps (filenames) to add
	 * @param add_mode Controls whether to activate maps that have low priority, or had previously been disabled by the user
	 * @param force_reload If true, plugin maps are always reloaded, even if no maps were added
	 * @param suppress_reload If true (and force_reload is false), do not reload plugin maps, even if maps were added
	 */
	static void registerPluginMaps (const QStringList &maps, AddMode add_mode, bool force_reload, bool suppress_reload=false);
	/** Looks for the given id among known plugin maps */
	static QString findPluginMapById (const QString &id);

	enum PluginMapPriority { PriorityHidden = 0, PriorityLow, PriorityMedium, PriorityHigh };
	enum PluginMapState { Broken, Quirky, Working };
	/** Marks given map as working, quirky, or broken. @returns true, if the state has changed. Broken maps are also disabled. */
	static bool markPluginMapState(const QString &map, PluginMapState state);

	/** Helper struct used by RKSettingsModulePlugins to keep track of plugin map files. */
	struct PluginMapStoredInfo {
		explicit PluginMapStoredInfo(const QString &_filename) : filename(_filename), state(Working) {};
		QString filename;
		PluginMapState state;
		int priority;
		RKParsedVersion version;
		QString id;
		QDateTime last_modified;
		bool betterThan(const PluginMapStoredInfo &other) const;
	};
	typedef QList<PluginMapStoredInfo> PluginMapInfoList;
	class RKPluginMapList {
	public:
		/** @returns true if the _effective_ list changed (false, if file already known, or another file by the same id) */
		bool addMap(const PluginMapStoredInfo &inf, AddMode add_mode);
		void removeObsoleteMaps();
		bool setMapfileState(const QString &mapfile, PluginMapState state);
		void forgetId(const QString &id);
		void setIdActive(const QString &id, bool active);
		QString mapFileForId(const QString &id) const;
		PluginMapInfoList allUniqueMaps();
		QStringList activeMapFiles();
		void saveToConfig(KConfigGroup &cg);
		void readFromConfig(KConfigGroup &cg);
	private:
friend class RKSettingsModulePluginsModel;
		struct DummyListStruct {
			bool active;
			PluginMapInfoList list;
		};
		QHash<QString, DummyListStruct> all_maps;
		QStringList ordered_ids;
	};
	/** Registers the plugin maps that are shipped with RKWard.
	 * @param force_add All default maps are also activated, even if they were already known, and disabled by the user. */
	static void registerDefaultPluginMaps (AddMode add_mode);
public Q_SLOTS:
	void settingChanged ();
	void configurePluginmaps ();
private:
	QButtonGroup *button_group;

	/** plugin maps which are not necessarily active, but have been encountered, before. @see plugin_maps */
	static RKPluginMapList known_plugin_maps;
friend class RKSettingsModulePluginsModel;
	static RKPluginMapList knownPluginmaps() { return known_plugin_maps; };
	static PluginMapStoredInfo parsePluginMapBasics(const QString &filename);

	static RKConfigValue<PluginPrefs,int> interface_pref;
	static RKConfigValue<bool> show_code;
	static RKConfigValue<int> code_size;
	static RKConfigValue<int> side_preview_width;

/* TODO: This one is currently unused (leftover of GHNS-based plugin installation), but might still be of interest */
	static QStringList findPluginMapsRecursive (const QString &basedir);
friend class RKPluginMapSelectionWidget;
/** Sets the new list of plugins. Potentially removes unreadable ones, and returns the effective list. */
	static RKPluginMapList setPluginMaps (const RKPluginMapList &new_list);
};

class RKSettingsModulePluginsModel : public QAbstractTableModel {
	Q_OBJECT
public:
	explicit RKSettingsModulePluginsModel (QObject* parent);
	virtual ~RKSettingsModulePluginsModel ();
/** (re-)initialize the model */
	void init (const RKSettingsModulePlugins::RKPluginMapList &known_plugin_maps);
	RKSettingsModulePlugins::RKPluginMapList pluginMaps () { return plugin_maps; };
public Q_SLOTS:
	void swapRows (int rowa, int rowb);
	void insertNewStrings (int above_row);
private:
	RKSettingsModulePlugins::RKPluginMapList plugin_maps;
	struct PluginMapMetaInfo {
		RKComponentAboutData *about;
		QList<RKComponentDependency> dependencies;
	};
	QHash<QString, PluginMapMetaInfo> plugin_map_dynamic_info;
	const PluginMapMetaInfo &getPluginMapMetaInfo (const QString &pluginmapfile);

	// reimplemented model functions
	int rowCount (const QModelIndex &parent = QModelIndex()) const override;
	int columnCount (const QModelIndex &parent = QModelIndex()) const override;
	QVariant data (const QModelIndex &index, int role = Qt::DisplayRole) const override;
	QVariant headerData (int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	bool setData (const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    bool removeRows (int row, int count, const QModelIndex& parent = QModelIndex ()) override;
	Qt::ItemFlags flags (const QModelIndex &index) const override;
};

#endif
