/***************************************************************************
                          rkstandardcomponent  -  description
                             -------------------
    begin                : Sun Feb 19 2006
    copyright            : (C) 2006 by Thomas Friedrichsmeier
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

#ifndef RKSTANDARDCOMPONENT_H
#define RKSTANDARDCOMPONENT_H

#include "rkcomponent.h"

#include <qdom.h>

class RKErrorDialog;
class RKStandardComponentGUI;
class ScriptBackend;

/** The standard type of component (i.e. stand-alone), previously known as "plugin". This is the type of component described by an XML-file */
class RKStandardComponent : public RKComponent {
	Q_OBJECT
public:
/** constructor.
@param parent_component Parent component (or 0, if this is going to be a top-level component)
@param parent_widget Parent widget (typically 0, if this is going to be a top-level component)
@param filename Filename of the XML-file to construct this component from */
	RKStandardComponent (RKComponent *parent_component, QWidget *parent_widget, const QString &filename);
/** destructor */
	~RKStandardComponent ();
/** reimplemented to update code on changes*/
	void changed ();
/** reimplemented to return true only when the backend is idle*/
	bool isReady ();
public slots:
	void switchInterfaces ();
/** this gets called by the script-backend, when it's done. Might enable the
	submit button or destruct the plugin. */
	void backendIdle ();
/** return result of given call (string vector) to the R-backend */	
//	void getRVector (const QString &call);
/** return result of given call to the R-backend */
//	void doRCall (const QString &call);
/** get a value for the backend */
	void getValue (const QString &id);
private:
/** The property holding the generated code. TODO: maybe, de facto, this property should be controlled (but not owned) by the scriptbackend. This way, we'd need less twisted logic inside this class. */
	RKComponentPropertyCode *code;
	QString filename;
	ScriptBackend *backend;
	RKErrorDialog *error_dialog;
	RKStandardComponentGUI *gui;
/** sometimes the plugin can't be destroyed immediately, since, for example the script-backend is
	still busy cleaning stuff up. In that case this var is set and the plugin gets destroyed ASAP. */
	bool created;
};

#include <qwidget.h>

class RKCommandEditor;
class QPushButton;
class QTimer;
class QSplitter;

/** contains the standard GUI elements for a top-level RKStandardComponent
TODO: differentiate into two classes for dialog and wizard interfaces. For now we ignore the wizard interface */
class RKStandardComponentGUI : public QWidget {
	Q_OBJECT
public:
	RKStandardComponentGUI (RKStandardComponent *component, RKComponentPropertyCode *code_property);
	~RKStandardComponentGUI ();

	QWidget *mainWidget () { return main_widget; };

	void enableSubmit (bool enable);
	void updateCode ();
public slots:
	void ok ();
	void back ();
	void cancel ();
	void toggleCode ();
	void help ();
	void codeChanged (RKComponentPropertyBase *);
	void updateCodeNow ();
private:
	QWidget *main_widget;
	RKComponentPropertyCode *code_property;
	QTimer *code_update_timer;

	// standard gui-elements
	RKCommandEditor *code_display;

	// common widgets
	QSplitter *splitter;
	QPushButton *ok_button;
	QPushButton *cancel_button;
	QPushButton *help_button;
	QPushButton *switch_button;

	// widgets for dialog only
	QPushButton *toggle_code_button;
private:
	RKStandardComponent *component;
protected:
	void closeEvent (QCloseEvent *e);
};

/** A helper class used to build and initialize an RKComponent. Most importantly this will keep track of the properties yet to be connected. Used at least by RKStandardComponent.

Notes: How does building work?
- Builder builds the components. Simple components are built by the same builder. For embedded components, a sub-builder is invoked.
- Simple components register their (property) connection wishes to the the builder during construction
- Builder takes care of connecting properties

Important: For built components with non-zero parent components should call parent_component->addChild () to register them! As an exception, this may be omitted for passive components (e.g. layouting components) that do not specify an id

Reminder to the twisted brain: Typically inside a standard-component, *all* child components, even if nested in layouting components, etc. have the same standard-component as parent! Only embedded full-fledged components are a truely separate unit!
*/
class RKComponentBuilder {
public:
	RKComponentBuilder (RKComponent *parent_component);
	~RKComponentBuilder ();
	void buildElement (const QDomElement &element, QWidget *parent_widget);
	void makeConnections ();
	RKComponent *component () const { return parent; };
private:
/** internal convenience function to schedule a property connection */
	void addConnection (const QString &client_id, const QString &client_property, const QString &governor_id, const QString &governor_property, bool reconcile, const QDomElement &origin);
	RKComponent *parent;
	struct RKComponentPropertyConnection {
		QString governor_property;
		QString client_property;
		bool reconcile;
		QDomElement origin;
	};
	typedef QValueList <RKComponentPropertyConnection> ConnectionList;
	ConnectionList connection_list;
};

#endif
