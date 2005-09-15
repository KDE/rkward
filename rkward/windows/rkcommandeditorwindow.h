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
	\brief Provides an editor window for R-commands, as well as a text-editor window in general.

While being called RKCommandEditorWindow, this class handles all sort of text-files, both read/write and read-only. It is an MDI child that is added to the main window, based on KatePart.

TODO: find out, whether save (), saveAs (), and some others are still needed. Clean up includes after that.
TODO: once ShowEditTextFileWindow is done, derive this class from that.

@author Pierre Ecochard
*/
class RKCommandEditorWindow : public KMdiChildView, public RCommandReceiver {
// we need the Q_OBJECT thing for some inherits ("RKCommandEditorWindow")-calls in rkward.cpp.
	Q_OBJECT
public:
    RKCommandEditorWindow (QWidget *parent = 0, bool use_r_highlighting=true);

    ~RKCommandEditorWindow();
    QString getSelection();
    QString getLine();
    QString getText();
    bool openURL(const KURL &url, bool use_r_highlighting=true, bool read_only=false);
    bool isModified();
    void insertText(QString text);
    /** Show help about the current word. */
    void showHelp();
    void rCommandDone (RCommand *command);
private:
	Kate::Document *m_doc;
	Kate::View *m_view;
	
	void setRHighlighting (Kate::Document *doc);
	bool getFilenameAndPath(const KURL &url,QString *fname);
private:
    void updateTabCaption(const KURL &url);
    RCommandChain *chain;
};

#endif
