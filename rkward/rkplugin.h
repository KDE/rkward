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

#include "rinterface.h"

class QDomElement;
class RKwardApp;
class QDialog;
class QBoxLayout;
class QGridLayout;
class QLayout;
class QWidgetStack;
class RKPluginWidget;
class QPushButton;
class RKVarSelector;
class PHPBackend;
class RCommand;
class RKErrorDialog;
class RKCommandEditor;

/** This class represents an activated plugin.

  *@author Thomas Friedrichsmeier
  */

class RKPlugin : public QWidget {
	Q_OBJECT
public: 
// TODO: pass directory only, not filename
	RKPlugin(RKwardApp *parent, const QString &filename);
	~RKPlugin();
//	QString tag () { return _tag; }
	RKwardApp *getApp () { return app; };
/** Returns a pointer to the varselector by that name (0 if not available) */
	RKVarSelector *getVarSelector (const QString &id);
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
/** Get result of r-command (which was requested for the PHP-backend */
	void gotRResult (RCommand *command);
private:
friend class PHPBackend;
friend class RKPluginGUIWidget;
	void updateCode (const QString &text);

/** return result of given call to the R-backend */
	void doRCall (const QString &call);
/** return result of given call (string vector) to the R-backend */	
	void getRVector (const QString &call);

/** this gets called by the PHP-backend, when it's done. Might enable the
	submit button or destruct the plugin. Returns true in the latter case. */
	bool backendIdle ();

/** try to destruct the plugin */
	void try_destruct ();

/** called by the PHP-backend, when new (final) output is available. Passed on
	via RKWardApp->RKOutputWindow */
	void newOutput ();
	
	RKwardApp *app;
/** sometimes the plugin can't be destroyed immediately, since, for example the PHP-backend is
	still busy cleaning stuff up. In that case this var is set and the plugin gets destroyed ASAP. */
	bool should_destruct;
	bool should_updatecode;
//	QString _tag;

	typedef QMap<QString, RKPluginWidget*> WidgetsMap;
	WidgetsMap widgets;

	typedef QMap<RKPluginWidget*, int> PageMap;
	PageMap page_map;
	
/** Called from activated (). builds the GUI */
	void buildDialog (const QDomElement &dialog_element, bool wizard_available);
	void buildWizard (const QDomElement &wizard_element, bool dialog_available);
	
	void buildStructure (const QDomElement &element, QBoxLayout *parent, QWidget *pwidget);
	
	void registerWidget (RKPluginWidget *widget, const QString &id, int page=0);

	QString current_code;
	QString filename;
	
	void buildGUI (int type_override);
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
		
	PHPBackend *backend;
	RKErrorDialog *error_dialog;
	RCommand::CommandChain *php_backend_chain;
protected:
	void closeEvent (QCloseEvent *e);
};

#endif
