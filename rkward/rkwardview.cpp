/***************************************************************************
                          rkwardview.cpp  -  description
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

// include files for Qt
#include <qprinter.h>
#include <qpainter.h>

// application specific includes
#include "rkwardview.h"
#include "rkwarddoc.h"
#include "rkward.h"

RKwardView::RKwardView(QWidget *parent, const char *name) : TwinTable (parent, name)
{
  setBackgroundMode(PaletteBase);
}

RKwardView::~RKwardView()
{
}

RKwardDoc *RKwardView::getDocument() const
{
  RKwardApp *theApp=(RKwardApp *) parentWidget();

  return theApp->getDocument();
}

void RKwardView::print(QPrinter *pPrinter)
{
  QPainter printpainter;
  printpainter.begin(pPrinter);
	
  // TODO: add your printing code here

  printpainter.end();
}
