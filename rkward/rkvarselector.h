/***************************************************************************
                          rkvarselector.h  -  description
                             -------------------
    begin                : Thu Nov 7 2002
    copyright            : (C) 2002 by Thomas Friedrichsmeier
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

#ifndef RKVARSELECTOR_H
#define RKVARSELECTOR_H

#include <rkpluginwidget.h>

#include <qlistview.h>

class QWidget;

/**
  *@author Thomas Friedrichsmeier
  */

class RKVarSelector : public QListView, public RKPluginWidget {
   Q_OBJECT
public: 
	RKVarSelector(QWidget *parent);
	~RKVarSelector();
protected:
	QWidget *widget () const;
};

#endif
