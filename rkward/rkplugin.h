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

#include <qstring.h>
#include <qobject.h>
#include <qmap.h>

class QDomElement;
class RKwardApp;
class QDialog;
class QBoxLayout;
class QLayout;
class RKPluginWidget;
class QTextEdit;
class QPushButton;
class RKVarSelector;
class PHPBackend;
class RCommand;
class RKErrorDialog;

/**
  *@author Thomas Friedrichsmeier
  */

class RKPlugin : public QObject {
	Q_OBJECT
public: 
// TODO: pass directory only, not filename
	RKPlugin(RKwardApp *parent, const QString &label, const QString &filename);
	~RKPlugin();
	QString label () { return _label; };	
//	QString tag () { return _tag; }
	RKwardApp *getApp () { return app; };
/** Returns a pointer to the varselector by that name (0 if not available) */
	RKVarSelector *getVarSelector (const QString &id);
/** return value given by identifier */
	QString getVar (const QString &id);
public slots:
/** Slot called, when the menu-item for this widget is selected. Responsible
	for creating the GUI. */
	void activated ();
	void ok ();
	void cancel ();
	void toggleCode ();
	void toggleWarn ();
	void help ();
/** Widgets, that if changed, require the code/problem-views to be updated,
	connect to this slot (or call it directly). Updates code/problem-views */
	void changed ();
/** Discards this plugin (i.e. cleans up stuff that is no longer needed until
	this plugin gets activated () again. */
	void discard ();
/** Get result of r-command (which was requested for the PHP-backend */
	void gotRResult (RCommand *command);
private:
friend class PHPBackend;
friend class RKPluginGUIWidget;
	void updateCode (const QString &text);

/** return result of given call to the R-backend */
	void doRCall (const QString &call);

/** this gets called by the PHP-backend, when it's done. Might enable the
	submit button or destruct the plugin. Returns true in the latter case. */
	bool backendIdle ();

/** try to destruct the plugin */
	void try_destruct ();

/** called by the PHP-backend, when new (final) output is available. Passed on
	via RKWardApp->RKOutputWindow */
	void newOutput ();
	
	QString filename;
	RKwardApp *app;
	QString _label;
/** sometimes the plugin can't be destroyed immediately, since, for example the PHP-backend is
	still busy cleaning stuff up. In that case this var is set and the plugin gets destroyed ASAP. */
	bool should_destruct;
	bool should_updatecode;
//	QString _tag;

	QWidget *gui;

	typedef QMap<QString, RKPluginWidget*> WidgetsMap;
	WidgetsMap widgets;

/** Called from activated (). builds the GUI */
	void buildGUI (const QDomElement &layout_element);

	void buildStructure (const QDomElement &element, QLayout *parent, QWidget *pwidget);

	// standard gui-elements
	QTextEdit *codeDisplay;
	QTextEdit *warnDisplay;

	QPushButton *okButton;
	QPushButton *cancelButton;
	QPushButton *helpButton;
	QPushButton *toggleCodeButton;
	QPushButton *toggleWarnButton;
	
	PHPBackend *backend;
	RKErrorDialog *error_dialog;
};

#endif
