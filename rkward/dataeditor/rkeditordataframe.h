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

/**
An RKEditor for data.frames.

@author Thomas Friedrichsmeier
*/
class RKEditorDataFrame : public TwinTable, public RCommandReceiver {
public:
    RKEditorDataFrame (QWidget *parent);

    ~RKEditorDataFrame ();

	void syncToR (RCommandChain *sync_chain);
	
private:
/// syncs the whole table.
	void pushTable (RCommandChain *sync_chain);
	RCommandChain *open_chain;
	
	struct PullCommandIdentifier {
		TwinTableMember *table;
		bool as_column;
		int offset_row;
		int offset_col;
		int length;
		bool get_data_table_next;
	};
	
	typedef QMap<RCommand*, PullCommandIdentifier*> PullMap;
	PullMap pull_map;
protected:
	void openObject (RObject *object);
	void rCommandDone (RCommand *command);
};

#endif
