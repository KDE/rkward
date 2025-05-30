/*
rkstandardcomponent - This file is part of the RKWard project. Created: Sun Feb 19 2006
SPDX-FileCopyrightText: 2006-2016 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKSTANDARDCOMPONENT_H
#define RKSTANDARDCOMPONENT_H

#include "rkcomponent.h"

#include <QList>
#include <qdom.h>

class RKComponentScriptingProxy;
class RKStandardComponentGUI;
class RCommandChain;
class RKComponentHandle;
class RKStandardComponentStack;
class ScriptBackend;
class QTimer;
class RKXMLGUIPreviewArea;
class RKPreviewManager;

/** The standard type of component (i.e. stand-alone), previously known as "plugin". This is the type of component described by an XML-file

@author Thomas Friedrichsmeier */
class RKStandardComponent : public RKComponent {
	Q_OBJECT
  public:
	/** constructor.
	@param parent_component Parent component (or 0, if this is going to be a top-level component)
	@param parent_widget Parent widget (typically 0, if this is going to be a top-level component)
	@param filename Filename of the XML-file to construct this component from */
	RKStandardComponent(RKComponent *parent_component, QWidget *parent_widget, const QString &filename, const QString &id);
	/** destructor */
	~RKStandardComponent();
	/** reimplemented to update code on changes*/
	void changed() override;
	/** reimplemented to return true, if the RKStandardComponent is in Wizard mode */
	bool isWizardish() override;
	/** reimplemented to actually answer the question (if in Wizard mode) */
	bool havePage(bool next) override;
	/** reimplemented to actually move the page (if in Wizard mode)  */
	void movePage(bool next) override;
	/** reimplemented to actually answer the question (if in Wizard mode) */
	bool currentPageSatisfied() override;
	/** for use by RKComponentBuilder to add a page to a wizardish component */
	RKComponent *addPage() override;
	/** reimplemented to actually register the component with the wizard */
	void addComponentToCurrentPage(RKComponent *component) override;
	/** switch from dialog to wizard or vice versa */
	void switchInterface();
	/** RTTI */
	int type() override { return ComponentStandard; };
	/** set the GUI caption (if this is a top-level gui) */
	void setCaption(const QString &caption);
	/** return the filename of the xml file */
	QString getFilename() const { return filename; };
	XMLHelper *getXmlHelper();
	QString getId() const { return id; };
	bool haveHelp() const { return have_help; };
	/** Submits the current code (by simulating a click on the ok button).
	@param in_chain The command chain to insert the command in (0 for regular command stack).
	@return false, if the plugin-code could not be submitted (e.g. plugin was not satisfied) */
	bool submit(RCommandChain *in_chain = nullptr);
	/** convenience access function: closes the corresponding GUI */
	void close();
	/** reimplemented to actually return Dead or Processing when appropriate */
	ComponentStatus recursiveStatus() override;

	RCommandChain *commandChain() const { return command_chain; };

	/** Return the GUI-scripting handler (creating it, if needed) */
	RKComponentScriptingProxy *scriptingProxy();

	RKXMLGUIPreviewArea *addDockedPreview(RKComponentPropertyBool *controller, const QString &label, RKPreviewManager *manager);
  Q_SIGNALS:
	void standardInitializationComplete();
  public Q_SLOTS:
	/** this gets called by the script-backend, when it's done. Might enable the
	    submit button or destruct the plugin. */
	void backendIdle();
	/** return result of given call (string vector) to the R-backend */
	//	void getRVector (const QString &call);
	/** return result of given call to the R-backend */
	//	void doRCall (const QString &call);
	/** get a value for the backend. Note: hint should be one of ValueTypeHint */
	void getValue(const QString &id, const int hint);
	/** reimplemented from QWidget to hide the gui if applicable */
	void hide();
	/** for enslaved components */
	void showGUI();
	void handleChange();
	void kill();

  private:
	/** The property holding the generated code. Note that this member is tightly controlled by the ScriptBackend */
	RKComponentPropertyCode *code;
	bool killed;
	QString filename;
	bool have_help; // TODO: replace by filename, once we use the help more
	ScriptBackend *backend;
	RKComponentScriptingProxy *scripting;
	RKStandardComponentGUI *gui;
	QString id;
	RKStandardComponentStack *wizard;
	bool update_pending;
	RCommandChain *command_chain;
	/** Avoid updating code-display, etc. until the component is fully created */
	bool created;
	bool createTopLevel(const QDomElement &doc_element, int force_mode = 0, bool enslaved = false);
	void buildAndInitialize(const QDomElement &doc_element, const QDomElement &gui_element, QWidget *parent_widget, bool build_wizard, bool enslaved = false);
	/** used during switchInterfaces () to discard child components, and delete gui if applicable */
	void discard();
	XMLHelper *xml;

  protected:
	friend class RKComponentBuilder;
	/** reimplemented for technical reasons. Additionally registers component children with the component stack if in wizard mode */
	void addChild(const QString &id, RKComponentBase *child) override;
};

#include <qmap.h>

/** A helper class used to build and initialize an RKComponent. Most importantly this will keep track of the properties yet to be connected. Used at least by RKStandardComponent.

Notes: How does building work?
- Builder builds the components. Simple components are built by the same builder. For embedded components, a sub-builder is invoked.
- Simple components register their (property) connection wishes to the builder during construction
- Builder takes care of connecting properties

Calls parent_component->addChild () for built child-components. As an exception, this may be omitted for passive components (e.g. layouting components) that do not specify an id

Reminder to the twisted brain: Typically inside a standard-component, *all* child components, even if nested in layouting components, etc. have the same standard-component as parent! Only embedded full-fledged components are a truly separate unit!

@author Thomas Friedrichsmeier */
class RKComponentBuilder {
  public:
	RKComponentBuilder(RKComponent *parent_component, const QDomElement &document_element);
	~RKComponentBuilder();
	void buildElement(const QDomElement &element, XMLHelper &xml, QWidget *parent_widget, bool allow_pages);
	void parseLogic(const QDomElement &element, XMLHelper &xml, bool allow_script_tag = true);
	void makeConnections();
	RKComponent *component() const { return parent; };

  private:
	/** internal convenience function to schedule a property connection */
	void addConnection(const QString &client_id, const QString &client_property, const QString &governor_id, const QString &governor_property, bool reconcile, const QDomElement &origin);
	QDomElement doElementCopy(const QString &id, XMLHelper &xml, const QDomElement &copy);
	QDomElement doc_elem;
	RKComponent *parent;
	struct RKComponentPropertyConnection {
		QString governor_property;
		QString client_property;
		bool reconcile;
		QDomElement origin;
	};
	typedef QList<RKComponentPropertyConnection> ConnectionList;
	ConnectionList connection_list;
	QMap<QString, QString> initial_values;
};

#endif
