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
class RKMenu;
class QDialog;
class QBoxLayout;
class QLayout;
class RKPluginWidget;
class QTextEdit;
class QPushButton;

/**
  *@author Thomas Friedrichsmeier
  */

class RKPlugin : public QObject {
	Q_OBJECT
public: 
	RKPlugin(RKMenu *parent, const QDomElement &element, QString filename);
	~RKPlugin();
	QString label () { return _label; };	
//	QString tag () { return _tag; }
public slots:
	void activated ();
	void ok ();
	void cancel ();
	void toggleCode ();
	void toggleWarn ();
	void help ();
private:
	QString filename;
	RKMenu *parent;
	QString _label;
//	QString _tag;

	QWidget *gui;

	QMap <QString, RKPluginWidget*> widgets;

	void buildGUI ();

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
