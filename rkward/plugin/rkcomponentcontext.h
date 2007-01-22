/***************************************************************************
                          rkcomponentcontext  -  description
                             -------------------
    begin                : Mon Jan 22 2007
    copyright            : (C) 2007 by Thomas Friedrichsmeier
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

#ifndef RKCOMPONENTCONTEXT_H
#define RKCOMPONENTCONTEXT_H

#include <qobject.h>

#include "rkcomponentmap.h"
#include "rkcomponent.h"

class QDomElement;
class RKContextHandler;

class RKContextMap : public RKComponentGUIXML {
public:
	RKContextMap ();
	~RKContextMap ();

	int create (const QDomElement &context_element, const QString &component_namespace);
	RKContextHandler *makeContextHandler (QObject *parent);
protected:
	void addedEntry (const QString &id, RKComponentHandle * /* handle */);
private:
	QStringList component_ids;
};



class RKContextHandler : public QObject, public RKComponentBase, public KXMLGUIClient {
	Q_OBJECT
friend class RKContextMap;
protected:
	RKContextHandler (QObject *parent, const QDomDocument &gui_xml);
	~RKContextHandler ();

	void addAction (const QString &id, RKStandardComponentHandle *handle);
private slots:
	void componentActionActivated ();
private:
	typedef QMap<const KAction *, RKStandardComponentHandle *> ActionMap;
	ActionMap action_map;
};

#endif
