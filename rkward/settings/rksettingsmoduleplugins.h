/***************************************************************************
                          rksettingsmoduleplugins  -  description
                             -------------------
    begin                : Wed Jul 28 2004
    copyright            : (C) 2004-2016 by Thomas Friedrichsmeier
    email                : thomas.friedrichsmeier@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
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
class QCheckBox;
class RKSpinBox;

/** The settings-module for plugin specific settings
@author Thomas Friedrichsmeier
*/
class RKSettingsModulePlugins : public RKSettingsModule {
	Q_OBJECT
public:
	RKSettingsModulePlugins (RKSettings *gui, QWidget *parent);
	~RKSettingsModulePlugins ();

	void applyChanges ();
	void save (KConfig *config);
	
	enum PluginPrefs { PreferDialog=0, PreferRecommended=1, PreferWizard=2 };
	
	static void saveSettings (KConfig *config);
	static void loadSettings (KConfig *config);
	
	QString caption ();

	/** @returns a list of active plugin maps */
	static QStringList pluginMaps ();
	static PluginPrefs getInterfacePreference () { return interface_pref; };
	static bool showCodeByDefault () { return show_code; };
	static void setShowCodeByDefault (bool shown) { show_code = shown; };
	static int defaultCodeHeight () { return code_size; };
	static void setDefaultCodeHeight (int new_height) { code_size = new_height; }
	static int defaultOtherPreviewHeight () { return other_preview_height; };
	static void setDefaultOtherPreviewHeight (int new_height) { other_preview_height = new_height; }
	/** register a list of available plugin-maps (which may or may not already be known). New maps are activated, automatically.
	 * @param maps Plugin maps (filenames) to add
	 * @param force_add If true, maps are added, even if they are not "new", and had previously been disabled by the user
	 * @param force_reload If true, plugin maps are always reloaded, even if no maps were added
	 * @param suppress_reload If true (and force_reload is false), do not reload plugin maps, even if maps were added
	 */
	static void registerPluginMaps (const QStringList &maps, bool force_add, bool force_reload, bool suppress_reload=false);
	/** Looks for the given id among known plugin maps */
	static QString findPluginMapById (const QString &id);
	/** marks given map as broken (in this version), and deactivates it. @Returns false is the map was already known to be broken, true otherwise. */
	static bool markPluginMapAsBroken (const QString &map);
	/** marks given map as quirky (in this version). @Returns false is the map was already known to be quirky, true otherwise. */
	static bool markPluginMapAsQuirky (const QString &map);
	/** Clears the broken or quirky flags. E.g. after the map was loaded, successfully */
	static void markPluginMapAsWorking (const QString &map);

	enum PluginMapPriority { PriorityHidden = 0, PriorityLow, PriorityMedium, PriorityHigh };
	/** Helper struct used by RKSettingsModulePlugins to keep track of plugin map files. */
	struct PluginMapStoredInfo {
		PluginMapStoredInfo (const QString &_filename) : filename (_filename), active (false), broken_in_this_version (false), quirky_in_this_version (false) {};
		QString filename;
		bool active;
		bool broken_in_this_version;
		bool quirky_in_this_version;
		int priority;
		QString id;
		QDateTime last_modified;
	};
	typedef QList<PluginMapStoredInfo> PluginMapList;
	static PluginMapList knownPluginmaps () { return known_plugin_maps; };
	static void parsePluginMapBasics (const QString &filename, QString *id, int *priority);
public slots:
	void settingChanged ();
	void configurePluginmaps ();
private:
	QButtonGroup *button_group;

	/** plugin maps which are not necessarily active, but have been encountered, before. @see plugin_maps */
	static PluginMapList known_plugin_maps;

	static PluginPrefs interface_pref;
	static bool show_code;
	static int code_size;
	static int other_preview_height;

/* TODO: This one is currently unused (leftover of GHNS-based plugin installation), but might still be of interest */
	static QStringList findPluginMapsRecursive (const QString &basedir);
	static void fixPluginMapLists ();
friend class RKPluginMapSelectionWidget;
/** Sets the new list of plugins. Potentially removes unreadable ones, and returns the effective list. */
	static PluginMapList setPluginMaps (const PluginMapList new_list);
};

class RKSettingsModulePluginsModel : public QAbstractTableModel {
	Q_OBJECT
public:
	explicit RKSettingsModulePluginsModel (QObject* parent);
	virtual ~RKSettingsModulePluginsModel ();
/** (re-)initialize the model */
	void init (const RKSettingsModulePlugins::PluginMapList &known_plugin_maps);
	RKSettingsModulePlugins::PluginMapList pluginMaps () { return plugin_maps; };
public slots:
	void swapRows (int rowa, int rowb);
	void insertNewStrings (int above_row);
private:
	RKSettingsModulePlugins::PluginMapList plugin_maps;
	struct PluginMapMetaInfo {
		RKComponentAboutData *about;
		QList<RKComponentDependency> dependencies;
	};
	QHash<QString, PluginMapMetaInfo> plugin_map_dynamic_info;
	const PluginMapMetaInfo &getPluginMapMetaInfo (const QString &pluginmapfile);

	// reimplemented model functions
	int rowCount (const QModelIndex &parent = QModelIndex()) const;
	int columnCount (const QModelIndex &parent = QModelIndex()) const;
	QVariant data (const QModelIndex &index, int role = Qt::DisplayRole) const;
	QVariant headerData (int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	bool setData (const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    bool removeRows (int row, int count, const QModelIndex& parent = QModelIndex ());
	Qt::ItemFlags flags (const QModelIndex &index) const;
};

#endif
