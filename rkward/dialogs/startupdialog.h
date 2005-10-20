/***************************************************************************
                          startupdialog  -  description
                             -------------------
    begin                : Thu Aug 26 2004
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
#ifndef STARTUPDIALOG_H
#define STARTUPDIALOG_H

#include <qdialog.h>

#include <kurl.h>

class QPushButton;
class QCheckBox;
class QButtonGroup;
class QRadioButton;
class QListView;
class QListViewItem;
class QPixmap;
class KRecentFilesAction;

/**
This class represents the startup dialog asking you whether to open a recent file, start with an empty workspace, etc. It's probably only ever used in one place, namely RKWard startup. The normal way to invoke it is via the static member-function getStartupAction. The selected action is returned in a StartupDialogResult. Remember to delete that struct after you read it.

@author Thomas Friedrichsmeier
*/
/// the startup dialog
class StartupDialog : public QDialog {
Q_OBJECT
public:
	/** enum to hold result of StartupDialog. WARNING: do not change the numeric values! They are saved as user settings in the config (@see RKSettingsModuleGeneral). */
	enum Result {
		EmptyWorkspace=0,	/**< start with an empty workspace */
		EmptyTable=1,				/**< start with an empty table */
		ChoseFile=2,				/**< chose file to open */
		NoSavedSetting=3,		/**< not acutally returned as a result. Used in RKSettingsModuleGeneral. This is saved, if user wants to be asked on every startup */
		OpenFile=4					/**< open a recent file (already specified) */
	};
	struct StartupDialogResult {
		Result result;
		KURL open_url;
	};

	StartupDialog (QWidget *parent, StartupDialogResult *result, KRecentFilesAction *recent_files);

	~StartupDialog();
	
	static StartupDialogResult *getStartupAction (QWidget *parent, KRecentFilesAction *recent_files);
public slots:
	void accept ();
	void reject ();
	void listDoubleClicked (QListViewItem *item, const QPoint &, int);
	void listClicked (QListViewItem *item);
	void openButtonSelected (int state);
private:
	QPushButton *ok_button;
	QPushButton *cancel_button;
	QRadioButton *empty_workspace_button;
	QRadioButton *empty_table_button;
	QRadioButton *open_button;
	QCheckBox *remember_box;
	
	QButtonGroup *choser;
	
	QListView *file_list;
	QListViewItem *chose_file_item;
	
	QPixmap *logo;
	
	StartupDialogResult *result;
};

#endif
