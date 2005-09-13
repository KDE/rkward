/***************************************************************************
                          showedittextfileagent  -  description
                             -------------------
    begin                : Tue Sep 13 2005
    copyright            : (C) 2005 by Thomas Friedrichsmeier
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

#include "showedittextfileagent.h"

ShowEditTextFileAgent::ShowEditTextFileAgent(QObject *parent, const char *name)
 : QObject(parent, name)
{
}


ShowEditTextFileAgent::~ShowEditTextFileAgent()
{
}


#include "showedittextfileagent.moc"
