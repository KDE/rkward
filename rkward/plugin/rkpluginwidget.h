/***************************************************************************
                          rkpluginwidget.h  -  description
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

#ifndef RKPLUGINWIDGET_H
#define RKPLUGINWIDGET_H

#include <qwidget.h>
#include <qstring.h>

class QLabel;
class QDomElement;
class RKPlugin;
class QLayout;

#define GENERIC_WIDGET 0

/** The baseclass for all RK-plugin-widgets, i.e. widgets, that are used in the
GUI for the plugins. It is mostly used as a skeleton and contains some virtual
functions to be implemented in the respective derived classes.
Note, that although being called a "widget", this class is actually a layout. This
is due to the fact, that most GUI-elements will want to lay themselves out in some
way, most importantly for including labels, but also for some more complex layouting
tasks. It's still called a "widget", because it essentially functions as one.

@author Thomas Friedrichsmeier
  */

class RKPluginWidget : public QWidget {
	Q_OBJECT
public: 
	RKPluginWidget (const QDomElement &element, QWidget *parent, RKPlugin *plugin);
	virtual ~RKPluginWidget();
/** Returns the plugin, this widget belongs to */	
	RKPlugin *plugin () { return _plugin; }
	QLabel *label;
/** Returns, whether the requirements for this widget are fulfilled.
	baseclass-implementation returns true. */
	virtual bool isSatisfied ();
/** Returns this widgets "value" */
	virtual QString value (const QString &modifier);
/** Returns any complaints, this widget may have.
	baseclass-implementation returns an empty string */
	virtual QString complaints ();
/** Returns, what type of widget this is */
	virtual int type () { return GENERIC_WIDGET; };
/** Performs any initialization that has to be delayed unitl all widgets are constructed (if any) */
	virtual void initialize () { ; };
signals:
	void changed ();
private:
	QWidget *_parent;
	RKPlugin *_plugin;
};

#endif
