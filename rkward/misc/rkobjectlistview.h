/*
rkobjectlistview - This file is part of the RKWard project. Created: Wed Sep 1 2004
SPDX-FileCopyrightText: 2004-2015 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKOBJECTLISTVIEW_H
#define RKOBJECTLISTVIEW_H

#include <QTreeView>
#include <QSortFilterProxyModel>

#include "../settings/rksettings.h"
#include "../core/robject.h"

class QMenu;
class RKListViewItem;
class RKObjectListViewSettings;
class QTimer;
class QCheckBox;
class QComboBox;
class QPushButton;
class RKDynamicSearchLine;

/**
This class provides the common functionality for the tree views in the RObjectBrowser and RKVarselector(s). The caps it (will) provide are: keeping the list up to date and emitting change-signals when appropriate, filtering for certain types of objects, sorting, mapping items to objects. Maybe some GUI-stuff like popup-menus should also be added to this class?

@author Thomas Friedrichsmeier
*/
class RKObjectListView : public QTreeView {
	Q_OBJECT
public:
	explicit RKObjectListView (bool toolwindow, QWidget *parent);
	~RKObjectListView ();

/** This function returns a pointer to the context menu of the RKObjectListView. It is provided so you can add your own items.
@returns a pointer to the context menu
@see aboutToShowContextMenu */
	QMenu *contextMenu () { return menu; };
/** This function returns the RObject the context menu has last been invoked on (or 0 if not invoked on an RObject). You can use this in slots called
from your custom menu items, to figure out, which object you should operate on. */
	RObject *menuObject () const { return menu_object; };

	RKObjectListViewSettings *getSettings () { return settings; };

/** Scrolls so that the item representing object becomes visible, and makes it current */
	void setObjectCurrent (RObject *object, bool only_if_none_current=false);
	void setRootObject (RObject *root);

	RObject::ObjectList selectedObjects () const;

	RObject* objectAtIndex (const QModelIndex& index) const;
/** Takes care initializing the RKObjectListView */
	void initialize ();
Q_SIGNALS:
/** This signal is emitted just before the context-menu is shown. If you connect to this signal, you can make some adjustments to the context-menu.
If you set *suppress to true, showing the context menu will be suppressed. */
	void aboutToShowContextMenu (RObject *object, bool *suppress);
public Q_SLOTS:
	void updateComplete ();
	void updateStarted ();
	void settingsChanged ();
	void itemClicked (const QModelIndex& index);
protected:
	void contextMenuEvent (QContextMenuEvent* event) override;
private:
	QMenu *menu;
	RObject *menu_object;
	RObject *root_object;
friend class RKObjectListViewRootDelegate;
	QAbstractItemDelegate *rkdelegate;
	RKObjectListViewSettings *settings;
};

/** Does filtering for an RKObjectListView. Should probably be renamed to RKObjectListViewFilter */
class RKObjectListViewSettings : public QSortFilterProxyModel {
	Q_OBJECT
public:
/** ctor. copies the default settings from RKSettingsModuleObjectBrowser */ 
	explicit RKObjectListViewSettings (bool toolwindow, QObject* parent=nullptr);
	~RKObjectListViewSettings ();

/** enum of @em persistent settings. There are more settings than these, but those will not be stored */
	enum PersistentSettings {
		ShowObjectsHidden,
		ShowFieldsType,
		ShowFieldsClass,
		ShowFieldsLabel,
		SettingsCount
	};

	void addSettingsToMenu (QMenu* menu, QAction* before);

	QWidget* filterWidget (QWidget *parent);
Q_SIGNALS:
	void settingsChanged ();
public Q_SLOTS:
	void filterSettingsChanged ();
	void updateSelfNow ();
	void resetFilters ();
protected:
	bool filterAcceptsRow (int source_row, const QModelIndex& source_parent) const override;
	bool acceptRow (int source_row, const QModelIndex& source_parent) const;
	bool filterAcceptsColumn (int source_column, const QModelIndex& source_parent) const override;
	bool lessThan (const QModelIndex& left, const QModelIndex& right) const override;
private:
	QAction* persistent_settings_actions[SettingsCount];
	bool persistent_settings[SettingsCount];

	void updateSelf ();

	QWidget *filter_widget;
	RKDynamicSearchLine *sline;
	QWidget *filter_widget_expansion;
	QCheckBox* filter_on_name_box;
	QCheckBox* filter_on_label_box;
	QCheckBox* filter_on_class_box;
	bool filter_on_name;
	bool filter_on_label;
	bool filter_on_class;
	QComboBox* depth_box;
	int depth_limit;
	QComboBox* type_box;
	bool hide_functions;
	bool hide_non_functions;
	QPushButton* reset_filters_button;
	bool in_reset_filters;

	bool is_tool_window;

	QTimer *update_timer;
};

#endif
