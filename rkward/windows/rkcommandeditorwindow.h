/***************************************************************************
                          rkcommandeditorwindow  -  description
                             -------------------
    begin                : Mon Aug 30 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
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
#ifndef RKCOMMANDEDITORWINDOW_H
#define RKCOMMANDEDITORWINDOW_H

#include <qwidget.h>
#include <qstring.h>


#include <kate/view.h>
#include <kate/document.h>
#include <kparts/mainwindow.h>
#include <kparts/partmanager.h>
#include <kparts/part.h>
#include <kparts/factory.h>

#include <kmdichildview.h>

#include <kurl.h>

class RKCommandEditor;
class KAction;
class KToggleAction;
class RKwardApp; 

/**
This class provides an editor window for R-commands. It is an MDI child that is added to the main window.

@author Pierre Ecochard
*/
class RKCommandEditorWindow : public KMdiChildView {
Q_OBJECT
public:
    RKCommandEditorWindow (QWidget *parent = 0);

    ~RKCommandEditorWindow();
    QString getSelection();
    QString getText();
    bool openURL(const KURL &url);
    KURL url();
    bool save();
    Kate::View *m_view;
    bool saveAs(const KURL &url);
    bool isModified();
    void cut();
    void copy();
    void paste();
    void undo();
    void redo();

private:
	Kate::Document *m_doc;
	   
	void setRHighlighting (Kate::Document *doc);
	KLibrary *m_library;
	bool getFilenameAndPath(const KURL &url,QString *fname);
	QBoxLayout* pLayout;
private slots:
    void slotGotFocus();
    void slotLostFocus();


};

#endif
