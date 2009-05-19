/***************************************************************************
                          rkstandardcomponent  -  description
                             -------------------
    begin                : Sun Feb 19 2006
    copyright            : (C) 2006, 2007, 2009 by Thomas Friedrichsmeier
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
#include <QList>

class RKStandardComponentGUI;
class RKComponentHandle;
class RKStandardComponentStack;
class ScriptBackend;
class QTimer;

/** The standard type of component (i.e. stand-alone), previously known as "plugin". This is the type of component described by an XML-file

@author Thomas Friedrichsmeier */
class RKStandardComponent : public RKComponent {
	Q_OBJECT
public:
/** constructor.
@param parent_component Parent component (or 0, if this is going to be a top-level component)
@param parent_widget Parent widget (typically 0, if this is going to be a top-level component)
@param filename Filename of the XML-file to construct this component from */
	RKStandardComponent (RKComponent *parent_component, QWidget *parent_widget, const QString &filename, RKComponentHandle *handle);
/** destructor */
	~RKStandardComponent ();
/** reimplemented to update code on changes*/
	void changed ();
/** reimplemented to return true only when the backend is idle */
	bool isReady ();
/** reimplemented to return true, if the RKStandardComponent is in Wizard mode */
	bool isWizardish ();
/** reimplemented to actually answer the question (if in Wizard mode) */
	bool havePage (bool next);
/** reimplemented to actually move the page (if in Wizard mode)  */
	void movePage (bool next);
/** reimplemented to actually answer the question (if in Wizard mode) */
	bool currentPageSatisfied ();
/** for use by RKComponentBuilder to add a page to a wizardish component */
	RKComponent *addPage ();
/** reimplemented to acutally register the component with the wizard */
	void addComponentToCurrentPage (RKComponent *component);
/** switch from dialog to wizard or vice versa */
	void switchInterface ();
/** RTTI */
	int type () { return ComponentStandard; };
/** set the GUI caption (if this is a top-level gui) */
	void setCaption (const QString &caption);
/** return the filename of the xml file */
	QString getFilename () { return filename; };
	RKComponentHandle *getHandle () { return handle; };
	bool haveHelp () { return have_help; };
/** tries to submit. Warning: This function waits for all changes to come in and may not return immediately!
@param max_wait Maximum time to wait for changes to settle in msecs (approx.)
@return true, if the plugin-code could be submitted */
	bool submit (int max_wait=1000);
/** convenience access function: closes the corresponding GUI */
	void close ();
public slots:
/** this gets called by the script-backend, when it's done. Might enable the
	submit button or destruct the plugin. */
	void backendIdle ();
/** return result of given call (string vector) to the R-backend */	
//	void getRVector (const QString &call);
/** return result of given call to the R-backend */
//	void doRCall (const QString &call);
/** get a value for the backend */
	void getValue (const QString &id);
/** reimplemented from QWidget to hide the gui if applicable */
	void hide ();
/** for enslaved components */
	void showGUI ();
	void handleChange ();
private:
/** The property holding the generated code. Note that this member is tightly controlled by the ScriptBackend */
	RKComponentPropertyCode *code;
	QString filename;
	bool have_help;	// TODO: replace by filename, once we use the help more
	ScriptBackend *backend;
	RKStandardComponentGUI *gui;
	RKComponentHandle *handle;
	RKStandardComponentStack *wizard;
	QTimer *handle_change_timer;
/** Avoid updating code-display, etc. until the component is fully created */
	bool created;
	bool createTopLevel (const QDomElement &doc_element, int force_mode=0, bool enslaved=false);
	void buildAndInitialize (const QDomElement &doc_element, const QDomElement &gui_element, QWidget *parent_widget, bool build_wizard, bool enslaved=false);
/** used during switchInterfaces () to discard child components, and delete gui if applicable */
	void discard ();
protected:
	friend class RKComponentBuilder;
/** reimplemented for technical reasons. Additionally registers component children with the component stack if in wizard mode */
	void addChild (const QString &id, RKComponentBase *child);
};

#include <qmap.h>

/** A helper class used to build and initialize an RKComponent. Most importantly this will keep track of the properties yet to be connected. Used at least by RKStandardComponent.

Notes: How does building work?
- Builder builds the components. Simple components are built by the same builder. For embedded components, a sub-builder is invoked.
- Simple components register their (property) connection wishes to the the builder during construction
- Builder takes care of connecting properties

Calls parent_component->addChild () for built child-components. As an exception, this may be omitted for passive components (e.g. layouting components) that do not specify an id

Reminder to the twisted brain: Typically inside a standard-component, *all* child components, even if nested in layouting components, etc. have the same standard-component as parent! Only embedded full-fledged components are a truly separate unit!

@author Thomas Friedrichsmeier */
class RKComponentBuilder {
public:
	RKComponentBuilder (RKStandardComponent *parent_component, const QDomElement &document_element);
	~RKComponentBuilder ();
	void buildElement (const QDomElement &element, QWidget *parent_widget, bool allow_pages);
	void parseLogic (const QDomElement &element);
	void makeConnections ();
	RKStandardComponent *component () const { return parent; };
private:
/** internal convenience function to schedule a property connection */
	void addConnection (const QString &client_id, const QString &client_property, const QString &governor_id, const QString &governor_property, bool reconcile, const QDomElement &origin);
	QDomElement doElementCopy (const QString id, const QDomElement &copy);
	QDomElement doc_elem;
	RKStandardComponent *parent;
	struct RKComponentPropertyConnection {
		QString governor_property;
		QString client_property;
		bool reconcile;
		QDomElement origin;
	};
	typedef QList <RKComponentPropertyConnection> ConnectionList;
	ConnectionList connection_list;
	QMap<QString, QString> initial_values;
};

#endif
