/***************************************************************************
                          rkward.h  -  description
                             -------------------
    begin                : Tue Oct 29 20:06:08 CET 2002 
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

#ifndef KHELPDLG_H
#define KHELPDLG_H

#include "rbackend/rcommandreceiver.h"

#include "helpdlg.h"

class RCommandChain;

class KHelpDlg : public helpDlg, public RCommandReceiver
{
  Q_OBJECT

public:
    KHelpDlg(QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~KHelpDlg();
    void rCommandDone (RCommand *command);
  /*$PUBLIC_FUNCTIONS$*/

public slots:
  /*$PUBLIC_SLOTS$*/
  virtual void          slotFindButtonClicked();
  virtual void          slotResultsListDblClicked( QListViewItem *item, const QPoint &, int );
  
protected:
  /*$PROTECTED_FUNCTIONS$*/

protected slots:
  /*$PROTECTED_SLOTS$*/
private:
    RCommandChain *chain;
};

#endif

