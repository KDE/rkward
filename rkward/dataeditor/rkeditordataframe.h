/***************************************************************************
                          rkeditordataframe  -  description
                             -------------------
    begin                : Fri Aug 20 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
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
#ifndef RKEDITORDATAFRAME_H
#define RKEDITORDATAFRAME_H

#include "rkeditor.h"
#include "twintable.h"
#include "../rbackend/rcommandreceiver.h"

class TwinTable;
class RCommand;
class RKVariable;

/**
An RKEditor for data.frames.

@author Thomas Friedrichsmeier
*/
class RKEditorDataFrame : public TwinTable, public RCommandReceiver {
	Q_OBJECT
public:
    RKEditorDataFrame (QWidget *parent);

    ~RKEditorDataFrame ();

	void syncToR (RCommandChain *sync_chain);
	
	void setColObject (int column, RObject *object);
	RObject *getColObject (int col);
	void objectDeleted (RObject *object);
	void objectMetaModified (RObject *object);
public slots:
	void metaValueChanged (int row, int col);
	void dataValueChanged (int row, int col);
	void columnDeleted (int col);
private:
/// syncs the whole table.
	void pushTable (RCommandChain *sync_chain);
	RCommandChain *open_chain;
	int getObjectCol (RObject *object);
	
	typedef QMap <int, RObject*> ColMap;
	ColMap col_map;
	
	void modifyObjectMeta (RKVariable *object, int column);
protected:
	void openObject (RObject *object);
	void rCommandDone (RCommand *command);
};

#endif
