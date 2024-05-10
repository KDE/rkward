/*
rkoptionset - This file is part of the RKWard project. Created: Mon Oct 31 2011
SPDX-FileCopyrightText: 2011-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKOPTIONSET_H
#define RKOPTIONSET_H

#include <rkcomponent.h>

#include <qmap.h>
#include <QDomElement>
#include <QTimer>
#include <QSet>
#include <QAbstractTableModel>

class RKAccordionTable;
class QTreeView;
class QPushButton;
class RKOptionSetDisplayModel;

/** An RKOptionSet provides a group of options for an arbitrary number of "rows". E.g. different line colors for each of a group of variables.
 * 
 * @author Thomas Friedrichsmeier
 */
class RKOptionSet : public RKComponent {
	Q_OBJECT
public:
	RKOptionSet (const QDomElement &element, RKComponent *parent_component, QWidget *parent_widget);
	~RKOptionSet ();
	int type () override { return ComponentOptionSet; };
	bool isValid () override;
	/** reimplemented from RKComponent */
	ComponentStatus recursiveStatus () override;
	/** reimplemented from RKComponent */
	void changed () override;
private Q_SLOTS:
	void governingPropertyChanged (RKComponentPropertyBase *property);
	void columnPropertyChanged (RKComponentPropertyBase *property);
	void currentRowPropertyChanged (RKComponentPropertyBase *property);
	void serializationPropertyChanged (RKComponentPropertyBase *property);
	void addRow (int where);
	void removeRow (int which);
	void currentRowChanged (int row);
	void fetchDefaults ();
	void slotUpdateUnfinishedRows ();
/** When keys in the key column change, all other columns have to be updated, accordingly. */
	void handleKeycolumnUpdate ();
protected:
friend class RKOptionSetDelegate;
	void fetchPropertyValuesRecursive (PropertyValueMap *list, bool include_top_level=false, const QString &prefix=QString (), bool include_inactive_elements=false) const override;
friend class RKOptionSetDisplayModel;
	int rowCount () const { return row_count->intValue (); };
	void setRowState (int row, bool finished, bool valid);
	void storeRowSerialization (int row);
	void applyContentsFromExternalColumn (RKComponentPropertyStringList* column, int row);
	void moveRow (int old_index, int new_index);

	RKComponentPropertyInt *current_row;
	RKComponentPropertyInt *row_count;
/** Un-serializing an optionset's state is terribly complicated, if it isn't guaranteed to happen in a single batch. Therefore, we
 * keep a dedicated property (serialization_of_set), which holds a _full_ serialization of the set's state. 
 * However, this representation is not kept up to date, for performance reasons. Rather it is generated only in fetchPropertyValuesRecursive(). */
 	RKComponentPropertyBase *serialization_of_set;

/** for option sets which are "driven" (i.e. the user cannot simply add / remove rows, directly), this holds the key column, controlling addition / removal of rows in the set.
  * if this length (or order) is changed in this row, it will also be changed in the other rows. */
	RKComponentPropertyStringList *keycolumn;
	QStringList old_keys;

	/** Map of properties (in the contents region) to columns which need to be updated, when the property changes. */
	QMultiMap<RKComponentPropertyBase *, RKComponentPropertyStringList *> columns_to_update;
	struct ColumnInfo {
		QString column_name;
		QString column_label;
		QString governor;
		QString governor_modifier;
		QString default_value;
		int display_index;
		bool external;
	};
	/** Map of all columns to their meta info */
	QMap<RKComponentPropertyStringList *, ColumnInfo> column_map;
	QList<RKComponentPropertyStringList*> visible_columns;
	struct RowInfo {
		explicit RowInfo(PropertyValueMap initial_values) : valid(false), finished(false), full_row_map(initial_values) {};
		bool valid;		/**< has finished processing and is known to be valid */
		bool finished;	/**< has finished processing */
		PropertyValueMap full_row_map;	/**< complete status representation of this row, (see RKComponent::fetchPropertyValuesRecursive()) */
	};
	QList<RowInfo> rows;
	PropertyValueMap default_row_state;
	int n_unfinished_rows, n_invalid_rows;
	int active_row;
	/** backup of row state info for rows corresponding to keys which have been removed (in a driven set). These might get re-inserted, later. */
	QHash<QString, PropertyValueMap> former_row_states;

	RKComponent *contents_container;
	ComponentStatus last_known_status;

	RKOptionSetDisplayModel* model;
	RKAccordionTable *accordion;

	QWidget *updating_notice;
	void updateUnfinishedRows ();
	int return_to_row;
	QTimer update_timer;

	int min_rows;
	int min_rows_if_any;
	int max_rows;

	bool updating;
/** Sets the contents from the values in given row */
	void setContentsForRow (int row);
	void updateCurrentRowInDisplay ();

/** get the default value for the given column, row. */
	friend QString getDefaultValue (const ColumnInfo& ci, int row);
};

class RKOptionSetDisplayModel : public QAbstractTableModel {
	Q_OBJECT
private:
friend class RKOptionSet;
	explicit RKOptionSetDisplayModel (RKOptionSet* parent);
	virtual ~RKOptionSetDisplayModel ();
	int rowCount (const QModelIndex & parent = QModelIndex()) const override;
	int columnCount (const QModelIndex & parent = QModelIndex()) const override;
	QVariant data (const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QVariant headerData (int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	void triggerReset ();
	QTimer reset_timer;
	QStringList column_labels;
	RKOptionSet *set;

	QMimeData* mimeData (const QModelIndexList& indexes) const override;
	QStringList mimeTypes () const override;
	bool dropMimeData (const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;
	Qt::ItemFlags flags (const QModelIndex& index) const override;
	Qt::DropActions supportedDropActions () const override;
	Qt::DropActions supportedDragActions () const override;
private Q_SLOTS:
	void doResetNow ();
};

#endif
