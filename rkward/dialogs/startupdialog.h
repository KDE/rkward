/*
startupdialog - This file is part of the RKWard project. Created: Thu Aug 26 2004
SPDX-FileCopyrightText: 2004-2011 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef STARTUPDIALOG_H
#define STARTUPDIALOG_H

#include <QDialog>
#include <QUrl>

class QCheckBox;
class QButtonGroup;
class QRadioButton;
class QListWidget;
class QListWidgetItem;
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
		EmptyWorkspace=0,			/**< start with an empty workspace */
		EmptyTable=1,				/**< start with an empty table */
		ChoseFile=2,				/**< chose file to open */
		NoSavedSetting=3,			/**< not actually returned as a result. Used in RKSettingsModuleGeneral. This is saved, if user wants to be asked on every startup */
		OpenFile=4,					/**< open a recent file (already specified) */
		RestoreFromWD=5				/**< load workspace from current directory, if available (R option --restore). */
	};
	struct StartupDialogResult {
		Result result;
		QUrl open_url;
	};

	StartupDialog (QWidget *parent, StartupDialogResult *result, KRecentFilesAction *recent_files);

	~StartupDialog();
	
	static StartupDialogResult getStartupAction (QWidget *parent, KRecentFilesAction *recent_files);
	static QUrl getRestoreFile ();
public slots:
	void accept () override;
	void reject () override;
	void listDoubleClicked (QListWidgetItem* item);
	void listClicked (QListWidgetItem* item);
	void openButtonSelected (bool checked);
protected:
/** reimplemented from QWidget to achieve fixed width */
	void showEvent (QShowEvent *event) override;
private:
	QRadioButton *empty_workspace_button;
	QRadioButton *empty_table_button;
	QRadioButton *restore_workspace_button;
	QRadioButton *open_button;
	QCheckBox *remember_box;
	
	QButtonGroup *choser;
	
	QListWidget *file_list;
	QListWidgetItem *chose_file_item;
	
	StartupDialogResult *result;
};

#endif
