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

/**
  *@author Thomas Friedrichsmeier
  */

class RKPlugin : public QObject {
	Q_OBJECT
public: 
	RKPlugin(RKwardApp *parent, const QDomElement &element, QString filename);
	~RKPlugin();
	QString label () { return _label; };	
//	QString tag () { return _tag; }
	RKwardApp *getApp () { return app; };
/** Returns a pointer to the varselector by that name (0 if not available) */
	RKVarSelector *getVarSelector (const QString &id);
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
private:
	QString filename;
	RKwardApp *app;
	QString _label;
//	QString _tag;

/** The code template as specified in the plugin's XML-description */
	QString code_template;

	QWidget *gui;

	QMap <QString, RKPluginWidget*> widgets;

/** Called from activated (). builds the GUI */
	void buildGUI (const QDomElement &layout_element);

	QBoxLayout *buildStructure (const QDomElement &element);
	RKPluginWidget *buildWidget (const QDomElement &element);

	// standard gui-elements
	QTextEdit *codeDisplay;
	QTextEdit *warnDisplay;

	QPushButton *okButton;
	QPushButton *cancelButton;
	QPushButton *helpButton;
	QPushButton *toggleCodeButton;
	QPushButton *toggleWarnButton;
};

#endif
