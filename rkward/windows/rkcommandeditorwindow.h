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

#include <kmdichildview.h>

#include "../rbackend/rcommandreceiver.h"

#include <kurl.h>

class RKCommandEditor;
class KAction;
class KToggleAction;
class RKwardApp; 
class RCommandChain;

/**
	\brief Provides an editor window for R-commands.

This is an MDI child that is added to the main window.

@author Pierre Ecochard
*/
class RKCommandEditorWindow : public KMdiChildView, public RCommandReceiver {
public:
    RKCommandEditorWindow (QWidget *parent = 0);

    ~RKCommandEditorWindow();
    QString getSelection();
    QString getLine();
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
    void insertText(QString text);
    /** Show help about the current word. */
    void showHelp();
    void rCommandDone (RCommand *command);
private:
	Kate::Document *m_doc;
	
	void setRHighlighting (Kate::Document *doc);
	bool getFilenameAndPath(const KURL &url,QString *fname);
private:
    void updateTabCaption(const KURL &url);
    RCommandChain *chain;
};

#endif
