/*
rkvareditmodel - This file is part of the RKWard project. Created: Mon Nov 05 2007
SPDX-FileCopyrightText: 2007-2011 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKVAREDITMODEL
#define RKVAREDITMODEL

#include <QAbstractTableModel>
#include <QItemSelectionRange>
#include <QList>

#include "../core/rkmodificationtracker.h"
#include "../core/rkvariable.h"
#include "rktextmatrix.h"

class RKVarEditMetaModel;
class RCommandChain;
class RKEditor;
class RKRowNames;

/** Base class for RKVarEditModel and RKVarEditMetaModel. Defines a common interface for copy and paste operations. Models might reimplement these functions for more efficiency.
@author Thomas Friedrichsmeier */
class RKVarEditModelBase : public QAbstractTableModel {
  public:
	explicit RKVarEditModelBase(QObject *parent) : QAbstractTableModel(parent), var_col_offset(0){};
	virtual ~RKVarEditModelBase(){};

	virtual RKTextMatrix getTextMatrix(const QItemSelectionRange &range) const = 0;
	virtual void blankRange(const QItemSelectionRange &range) = 0;
	virtual void setTextMatrix(const QModelIndex &offset, const RKTextMatrix &text, const QItemSelectionRange &confine_to = QItemSelectionRange()) = 0;

	virtual int trueRows() const = 0;
	virtual int trueCols() const = 0;
	int firstRealColumn() const { return var_col_offset; };

	int var_col_offset;
};

/** This class represents a collection of RKVariables of uniform length (typically a data.frame) suitable for editing in a model/view editor such as QTableView. Probably it will only ever support editing a single RKVariable, though, as it is not possible to ensure uniform length outside of a data.frame. For a data.frame use RKVarEditDataFrameModel . Since the real data storage is in RKVariable, it is ok (and recommended) to create separate models for separate editors/viewers, even if the objects in question are the same. */
class RKVarEditModel : public RKVarEditModelBase, public RObjectListener {
	Q_OBJECT
  public:
	explicit RKVarEditModel(QObject *parent);
	~RKVarEditModel();

	/** set the editor that is using this model. This is useful to find out, e.g. which window should be raised, when calling "Edit" on an object represented in this data-model. Also, the editor will be notified, if all objects in the model have been removed. */
	void setEditor(RKEditor *editor) { myeditor = editor; };
	/** see setEditor () */
	RKEditor *getEditor() const { return myeditor; };

	/** Add an object to the model at the given index. Calls this only once, unless from an RKVarEditDataFrameModel. You should add at least one object, **before** you add this model to any view.
	@param index position to insert at, or -1 to append the item
	@param object the objects to insert */
	void addObject(int index, RKVariable *object);
	/** Return a pointer to the meta model. The meta model is created the first time this function is called. */
	RKVarEditMetaModel *getMetaModel();

	// QAbstractTableModel implementations
	bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
	bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	Qt::ItemFlags flags(const QModelIndex &index) const override;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	RKTextMatrix getTextMatrix(const QItemSelectionRange &range) const override;
	void blankRange(const QItemSelectionRange &range) override;
	void setTextMatrix(const QModelIndex &offset, const RKTextMatrix &text, const QItemSelectionRange &confine_to = QItemSelectionRange()) override;

	int trueCols() const override { return objects.size(); };
	int trueRows() const override { return (objects.isEmpty() ? 0 : objects[0]->getLength()); };
	void lockHeader(bool lock) { header_locked = lock; };

	virtual void restoreObject(RObject *object, RCommandChain *chain);

	void objectMetaChanged(RObject *changed) override;
	void objectDataChanged(RObject *object, const RObject::ChangeSet *changes) override;

	RKVariable *getObject(int index) const;
	QStringList problems() const { return problem_details; };
  Q_SIGNALS:
	void modelDepleted();
	void hasProblems();
  private Q_SLOTS:
	void doResetNow();

  private:
	bool reset_scheduled;
	void scheduleReset();

  protected:
	friend class RKVarEditMetaModel;
	QList<RKVariable *> objects;
	QStringList problem_details;
	RKRowNames *rownames;
	void addProblem(const QString &desc);

	/** very simple convenience function to return the number of true cols + trailing cols */
	int apparentCols() const { return (trueCols() + trailing_cols); };
	/** very simple convenience function to return the number of true rows + trailing rows */
	int apparentRows() const { return (trueRows() + trailing_rows); };

	/** Receives notifications of object removals. Takes care of removing the object from the list. */
	void objectRemoved(RObject *object) override;

	/** insert new columns at index. Default implementation does nothing. To be implemented in subclasses */
	virtual void doInsertColumns(int index, int count);

	virtual void doInsertRowsInBackend(int row, int count);
	virtual void doRemoveRowsInBackend(int row, int count);

	int trailing_rows;
	int trailing_cols;

	int edit_blocks;
	bool header_locked;

	RKVarEditMetaModel *meta_model;
	RKEditor *myeditor;
};

/** Represents the meta information portion belonging to an RKVarEditModel. Implemented in a separate class for technical reasons, only (so this info can be displayed in a separate QTableView). This model mostly acts as a slave of an RKVarEditModel. You will not need to call any functions directly except from the RKVarEditModel, or an item view. */
class RKVarEditMetaModel : public RKVarEditModelBase {
	Q_OBJECT
  public:
	enum Rows {
		NameRow = 0,
		LabelRow,
		TypeRow,
		FormatRow,
		LevelsRow,
		RowCount = LevelsRow + 1
	};

	// QAbstractTableModel implementations
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	Qt::ItemFlags flags(const QModelIndex &index) const override;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	RKTextMatrix getTextMatrix(const QItemSelectionRange &range) const override;
	void blankRange(const QItemSelectionRange &range) override;
	void setTextMatrix(const QModelIndex &offset, const RKTextMatrix &text, const QItemSelectionRange &confine_to = QItemSelectionRange()) override;

	int trueCols() const override { return data_model->trueCols(); };
	int trueRows() const override { return RowCount; };

	RObject::ValueLabels getValueLabels(int column) const;
	void setValueLabels(int column, const RObject::ValueLabels &labels);

	RKVariable *getObject(int index) const { return data_model->getObject(index); };
	RKVarEditModel *getDataModel() const { return data_model; };

  protected:
	friend class RKVarEditModel;
	explicit RKVarEditMetaModel(RKVarEditModel *data_model);
	~RKVarEditMetaModel();

	void beginAddDataObject(int index);
	void endAddDataObject();
	void beginRemoveDataObject(int index);
	void endRemoveDataObject();
	void objectMetaChanged(int atcolumn);

	RKVarEditModel *data_model;
};

class RKVarEditDataFrameModel : public RKVarEditModel {
	Q_OBJECT
  public:
	RKVarEditDataFrameModel(RContainerObject *dataframe, QObject *parent);
	/** ctor that constructs a new empty data frame */
	RKVarEditDataFrameModel(const QString &validized_name, RContainerObject *parent_object, RCommandChain *chain, int initial_cols, QObject *parent);
	~RKVarEditDataFrameModel();

	bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;
	bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

	RContainerObject *getObject() const { return dataframe; };

	void restoreObject(RObject *object, RCommandChain *chain) override;
  Q_SIGNALS:
	void modelObjectDestroyed();

  protected:
	void doInsertColumns(int index, int count) override;
	/** reimplemented from RKVarEditModel to listen for the dataframe object as well */
	void objectRemoved(RObject *object) override;
	/** receives notifications of new objects added to this data.frame */
	void childAdded(int index, RObject *parent) override;
	/** receives notifications of object position changes inside this data.frame */
	void childMoved(int old_index, int new_index, RObject *parent) override;

	void doInsertRowsInBackend(int row, int count) override;
	void doRemoveRowsInBackend(int row, int count) override;

	RContainerObject *dataframe;

	void init(RContainerObject *dataframe);
	void pushTable(RCommandChain *sync_chain);
};

#endif
