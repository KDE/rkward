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
#ifndef RKHELPWINDOW_H
#define RKHELPWINDOW_H

#include <kurl.h>
#include <kparts/browserextension.h>

#include <kmdichildview.h>

class KHTMLPart;

/**
\brief Show html help.

This class wraps a khtml part.

@author Pierre Ecochard
*/
class RKHelpWindow : public KMdiChildView
{
Q_OBJECT
public:
    RKHelpWindow(QWidget *parent = 0, const char *name = 0);

    ~RKHelpWindow();
    bool openURL(KURL url);
public slots:
    void slotOpenURLRequest(const KURL &url, const KParts::URLArgs & );
private:
    KHTMLPart * khtmlpart;
    QBoxLayout* pLayout;
};

#endif
