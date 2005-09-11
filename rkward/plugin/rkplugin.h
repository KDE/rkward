/***************************************************************************
                          rkplugin.h  -  description
                             -------------------
    begin                : Wed Nov 6 2002
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

#ifndef RKPLUGIN_H
#define RKPLUGIN_H

#include <qwidget.h>

#include <qstring.h>
#include <qmap.h>

#include "../rbackend/rcommandreceiver.h"

class QDomElement;
class QDialog;
class QBoxLayout;
class QGridLayout;
class QLayout;
class QWidgetStack;
class RKPluginWidget;
class QPushButton;
class RKVarSelector;
class RKVarSlot;
class ScriptBackend;
class RCommand;
class RCommandChain;
class RKErrorDialog;
class RKCommandEditor;
class QTimer;

/** This class represents an activated plugin.

  *@author Thomas Friedrichsmeier
  */

class RKPlugin : public QWidget, public RCommandReceiver {
	Q_OBJECT
public: 
// TODO: pass directory only, not filename
	RKPlugin(const QString &filename);
	~RKPlugin();
//	QString tag () { return _tag; }
/** Returns a pointer to the varselector by that name (0 if not available) */
	RKVarSelector *getVarSelector (const QString &id);
/** Returns a pointer to the varslot by that name (0 if not available) */	
	RKVarSlot *getVarSlot (const QString &id);
/** return value given by identifier */
	QString getVar (const QString &id);
public slots:
	void ok ();
	void back ();
	void cancel ();
	void toggleCode ();
	void switchInterfaces ();
	void help ();
/** Widgets, that if changed, require the code/problem-views to be updated,
	connect to this slot (or call it directly). Updates code/problem-views */
	void changed ();
	
// slots below are called by the backend
/** this gets called by the script-backend, when it's done. Might enable the
	submit button or destruct the plugin. */
	void backendIdle ();
	void backendCommandDone (int flags);
/** return result of given call (string vector) to the R-backend */	
	void getRVector (const QString &call);
/** return result of given call to the R-backend */
	void doRCall (const QString &call);
/** get a value for the backend */
	void getValue (const QString &id);
	void doChangeUpdate ();
private:
	QTimer *update_timer;

/** try to destruct the plugin */
	void try_destruct ();
	
/** sometimes the plugin can't be destroyed immediately, since, for example the script-backend is
	still busy cleaning stuff up. In that case this var is set and the plugin gets destroyed ASAP. */
	bool should_destruct;
	bool should_updatecode;
//	QString _tag;

	typedef QMap<QString, RKPluginWidget*> WidgetsMap;
	WidgetsMap widgets;

	typedef QMap<RKPluginWidget*, int> PageMap;
	PageMap page_map;

  typedef QMap<QString, QString> Dependancies;
	Dependancies dependant;

/** Called from activated (). builds the GUI */
	void buildDialog (const QDomElement &dialog_element, bool wizard_available);
	void buildWizard (const QDomElement &wizard_element, bool dialog_available);
	
	void buildStructure (const QDomElement &element, QBoxLayout *parent, QWidget *pwidget);
	
	void registerWidget (RKPluginWidget *widget, const QString &id, const QString &dep, int page=0);

	QString current_code;
	QString filename;

/** if you pass 0 for doc_element, the description will be parsed anew */
	void buildGUI (QDomElement *doc_element, int type_override);
	QWidget *main_widget;
	QGridLayout *sizer_grid;

	// standard gui-elements
	RKCommandEditor *codeDisplay;

	// common widgets
	QPushButton *okButton;
	QPushButton *cancelButton;
	QPushButton *helpButton;
	QPushButton *switchButton;
	
	int num_pages;
	int current_page;
	
	// widgets for wizard only
	QWidgetStack *wizard_stack;
	QPushButton *backButton;

	// widgets for dialog only
	QPushButton *toggleCodeButton;
		
	ScriptBackend *backend;
	RKErrorDialog *error_dialog;
	RCommandChain *script_backend_chain;
protected:
	void closeEvent (QCloseEvent *e);
/** Get result of r-command (which was requested for the script-backend */
	void rCommandDone (RCommand *command);
};

#endif
