/***************************************************************************
                          rkobjectlistview  -  description
                             -------------------
    begin                : Wed Sep 1 2004
    copyright            : (C) 2004-2015 by Thomas Friedrichsmeier
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
#ifndef RKOBJECTLISTVIEW_H
#define RKOBJECTLISTVIEW_H

#include <QTreeView>

#include <krecursivefilterproxymodel.h>

#include "../settings/rksettings.h"
#include "../core/robject.h"

class QMenu;
class RKListViewItem;
class RKObjectListViewSettings;
class QActionGroup;
class QTimer;
class QCheckBox;

/**
This class provides the common functionality for the tree views in the RObjectBrowser and RKVarselector(s). The caps it (will) provide are: keeping the list up to date and emitting change-signals when appropriate, filtering for certain types of objects, sorting, mapping items to objects. Maybe some GUI-stuff like popup-menus should also be added to this class?

@author Thomas Friedrichsmeier
*/
class RKObjectListView : public QTreeView {
	Q_OBJECT
public:
	explicit RKObjectListView (QWidget *parent);
	
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
signals:
	void selectionChanged ();
/** This signal is emitted just before the context-menu is shown. If you connect to this signal, you can make some adjustments to the context-menu.
If you set *suppress to true, showing the context menu will be suppressed. */
	void aboutToShowContextMenu (RObject *object, bool *suppress);
public slots:
	void updateComplete ();
	void updateStarted ();
	void selectionChanged (const QItemSelection & selected, const QItemSelection & deselected);
	void settingsChanged ();
	void popupConfigure ();
	void itemClicked (const QModelIndex& index);
protected:
	void contextMenuEvent (QContextMenuEvent* event);
private:
	QMenu *menu;
	RObject *menu_object;
	RObject *root_object;
friend class RKObjectListViewRootDelegate;
	QAbstractItemDelegate *rkdelegate;
	RKObjectListViewSettings *settings;
};

/** Does filtering for an RKObjectListView. Should probably be renamed to RKObjectListViewFilter */
class RKObjectListViewSettings : public KRecursiveFilterProxyModel {
	Q_OBJECT
public:
/** ctor. copies the default settings from RKSettingsModuleObjectBrowser */ 
	explicit RKObjectListViewSettings (QObject* parent=0);
	~RKObjectListViewSettings ();

	enum Settings {
		ShowObjectsAllEnvironments,
		ShowObjectsHidden,
		ShowFieldsType,
		ShowFieldsClass,
		ShowFieldsLabel,
		SettingsCount
	};

	void setSetting (Settings setting, bool to);
	bool getSetting (Settings setting) const { return settings[setting].state; };

	QMenu *showObjectsMenu () const { return show_objects_menu; };
	QMenu *showFieldsMenu () const { return show_fields_menu; };

	QWidget* filterWidget (QWidget *parent);
signals:
	void settingsChanged ();
public slots:
	void globalSettingsChanged (RKSettings::SettingsPage);
	void filterSettingsChanged ();
	void settingToggled (QAction* which);
	void updateSelfNow ();
protected:
	bool acceptRow (int source_row, const QModelIndex& source_parent) const;
	bool filterAcceptsColumn (int source_column, const QModelIndex& source_parent) const;
	bool lessThan (const QModelIndex& left, const QModelIndex& right) const;
private:
	struct Setting {
		Setting () : is_at_default (true) {};
		QAction *action;
		bool is_at_default;
		bool state;
	};
	Setting settings[SettingsCount];
	QActionGroup* action_group;

	void createContextMenus ();
	void updateSelf ();

	QMenu *show_objects_menu;
	QMenu *show_fields_menu;

	QWidget *filter_widget;
	QWidget *filter_widget_expansion;
	QCheckBox* filter_on_name_box;
	QCheckBox* filter_on_label_box;
	QCheckBox* filter_on_class_box;
	bool filter_on_name;
	bool filter_on_label;
	bool filter_on_class;
	QCheckBox* show_variables_box;
	QCheckBox* show_containers_box;
	QCheckBox* show_functions_box;
	bool show_variables;
	bool show_containers;
	bool show_functions;
	QCheckBox* hidden_objects_box;
	bool show_hidden_objects;

	QTimer *update_timer;
};

#endif
