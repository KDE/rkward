/***************************************************************************
                          rksettingsmodulecommandeditor  -  description
                             -------------------
    begin                : Tue Oct 23 2007
    copyright            : (C) 2007, 2010, 2011 by Thomas Friedrichsmeier
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
#include "rksettingsmodulecommandeditor.h"

#include <klocale.h>
#include <kconfig.h>
#include <kconfiggroup.h>

#include <qlayout.h>
#include <qlabel.h>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QGroupBox>
#include <QLineEdit>

#include "../misc/rkspinbox.h"
#include "../misc/rkcommonfunctions.h"
#include "../rkglobals.h"
#include "../debug.h"

// static members
int RKSettingsModuleCommandEditor::completion_min_chars;
int RKSettingsModuleCommandEditor::completion_timeout;
bool RKSettingsModuleCommandEditor::completion_enabled;
bool RKSettingsModuleCommandEditor::arghinting_enabled;
bool RKSettingsModuleCommandEditor::autosave_enabled;
bool RKSettingsModuleCommandEditor::autosave_keep;
int RKSettingsModuleCommandEditor::autosave_interval;
int RKSettingsModuleCommandEditor::num_recent_files;
QString RKSettingsModuleCommandEditor::script_file_filter;

RKSettingsModuleCommandEditor::RKSettingsModuleCommandEditor (RKSettings *gui, QWidget *parent) : RKSettingsModule (gui, parent) {
	RK_TRACE (SETTINGS);

	QVBoxLayout* main_vbox = new QVBoxLayout (this);
	QLabel *label = new QLabel (i18n ("Settings marked with (*) do not take effect until you restart RKWard"), this);
	label->setWordWrap (true);
	main_vbox->addWidget (label);
	main_vbox->addSpacing (2 * RKGlobals::spacingHint ());

	QGroupBox* group = new QGroupBox (i18n ("Code Completion"), this);
	QVBoxLayout* box_layout = new QVBoxLayout (group);

	completion_enabled_box = new QCheckBox (i18n ("Enable code completion"), group);
	completion_enabled_box->setChecked (completion_enabled);
	connect (completion_enabled_box, SIGNAL (stateChanged(int)), this, SLOT (settingChanged()));
	box_layout->addWidget (completion_enabled_box);

	box_layout->addSpacing (RKGlobals::spacingHint ());

	label = new QLabel (i18n ("Minimum number of characters before completion is attempted"), group);
	label->setWordWrap (true);
	completion_min_chars_box = new RKSpinBox (group);
	completion_min_chars_box->setIntMode (1, INT_MAX, completion_min_chars);
	completion_min_chars_box->setEnabled (completion_enabled);
	connect (completion_min_chars_box, SIGNAL (valueChanged(int)), this, SLOT (settingChanged()));
	box_layout->addWidget (label);
	box_layout->addWidget (completion_min_chars_box);

	main_vbox->addSpacing (RKGlobals::spacingHint ());

	label = new QLabel (i18n ("Timeout (milli seconds) before completion is attempted"), group);
	label->setWordWrap (true);
	completion_timeout_box = new RKSpinBox (group);
	completion_timeout_box->setIntMode (0, INT_MAX, completion_timeout);
	completion_timeout_box->setEnabled (completion_enabled);
	connect (completion_timeout_box, SIGNAL (valueChanged(int)), this, SLOT (settingChanged()));
	box_layout->addWidget (label);
	box_layout->addWidget (completion_timeout_box);

	main_vbox->addWidget (group);

	arghinting_enabled_box = new QCheckBox (i18n ("Enable function argument hinting"), group);
	arghinting_enabled_box->setChecked (arghinting_enabled);
	connect (arghinting_enabled_box, SIGNAL (stateChanged(int)), this, SLOT (settingChanged()));
	main_vbox->addWidget (arghinting_enabled_box);

	main_vbox->addSpacing (2 * RKGlobals::spacingHint ());

	group = autosave_enabled_box = new QGroupBox (i18n ("Autosaves"), this);
	autosave_enabled_box->setCheckable (true);
	autosave_enabled_box->setChecked (autosave_enabled);
	connect (autosave_enabled_box, SIGNAL (toggled(bool)), this, SLOT (settingChanged()));
	box_layout = new QVBoxLayout (group);

	label = new QLabel (i18n ("Autosave interval (minutes)"), group);
	autosave_interval_box = new RKSpinBox (group);
	autosave_interval_box->setIntMode (1, INT_MAX, autosave_interval);
	connect (autosave_interval_box, SIGNAL (valueChanged(int)), this, SLOT (settingChanged()));
	box_layout->addWidget (label);
	box_layout->addWidget (autosave_interval_box);
	box_layout->addSpacing (RKGlobals::spacingHint ());

	autosave_keep_box = new QCheckBox (i18n ("Keep autosave file after manual save"), group);
	autosave_keep_box->setChecked (autosave_keep);
	connect (autosave_keep_box, SIGNAL (stateChanged(int)), this, SLOT (settingChanged()));
	box_layout->addWidget (autosave_keep_box);

	main_vbox->addWidget (group);

	main_vbox->addSpacing (2 * RKGlobals::spacingHint ());

	group = new QGroupBox (i18n ("Opening script files"), this);
	box_layout = new QVBoxLayout (group);
	label = new QLabel (i18n ("Number of scripts in recent file lists (*)"), group);
	num_recent_files_box = new RKSpinBox (group);
	num_recent_files_box->setIntMode (1, INT_MAX, num_recent_files);
	connect (num_recent_files_box, SIGNAL (valueChanged(int)), this, SLOT (settingChanged()));
	box_layout->addWidget (label);
	box_layout->addWidget (num_recent_files_box);
	box_layout->addSpacing (RKGlobals::spacingHint ());

	label = new QLabel (i18n ("R script file filters (separated by spaces)"), group);
	script_file_filter_box = new QLineEdit (group);
	RKCommonFunctions::setTips (i18n ("A list of filters (file name extensions) that should be treated as R script files. Most importantly, files matching one of these filters will always be opened with R syntax highlighting.<br>Filters are case insensitive."), script_file_filter_box, label);
	label->setToolTip (script_file_filter_box->toolTip ());
	connect (script_file_filter_box, SIGNAL (textChanged(QString)), this, SLOT (settingChanged()));
	box_layout->addWidget (label);
	box_layout->addWidget (script_file_filter_box);
	box_layout->addSpacing (RKGlobals::spacingHint ());

	main_vbox->addWidget (group);

	main_vbox->addStretch ();
}

RKSettingsModuleCommandEditor::~RKSettingsModuleCommandEditor () {
	RK_TRACE (SETTINGS);
}

void RKSettingsModuleCommandEditor::settingChanged () {
	RK_TRACE (SETTINGS);
	change ();

	completion_timeout_box->setEnabled (completion_enabled_box->isChecked ());
	completion_min_chars_box->setEnabled (completion_enabled_box->isChecked ());
}

QString RKSettingsModuleCommandEditor::caption () {
	RK_TRACE (SETTINGS);
	return (i18n ("Script editor"));
}

bool RKSettingsModuleCommandEditor::hasChanges () {
	RK_TRACE (SETTINGS);
	return changed;
}

void RKSettingsModuleCommandEditor::applyChanges () {
	RK_TRACE (SETTINGS);

	completion_enabled = completion_enabled_box->isChecked ();
	completion_min_chars = completion_min_chars_box->intValue ();
	completion_timeout = completion_timeout_box->intValue ();
	arghinting_enabled = arghinting_enabled_box->isChecked ();

	autosave_enabled = autosave_enabled_box->isChecked ();
	autosave_keep = autosave_keep_box->isChecked ();
	autosave_interval = autosave_interval_box->intValue ();

	num_recent_files = num_recent_files_box->intValue ();
	script_file_filter = script_file_filter_box->text ();
}

void RKSettingsModuleCommandEditor::save (KConfig *config) {
	RK_TRACE (SETTINGS);
	saveSettings (config);
}

void RKSettingsModuleCommandEditor::saveSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group ("Command Editor Windows");
	cg.writeEntry ("Completion enabled", completion_enabled);
	cg.writeEntry ("Completion min chars", completion_min_chars);
	cg.writeEntry ("Completion timeout", completion_timeout);
	cg.writeEntry ("Argument hinting enabled", arghinting_enabled);

	cg.writeEntry ("Autosave enabled", autosave_enabled);
	cg.writeEntry ("Autosave keep saves", autosave_keep);
	cg.writeEntry ("Autosave interval", autosave_interval);

	cg.writeEntry ("Max number of recent files", num_recent_files);
	cg.writeEntry ("Script file filter", script_file_filter);
}

void RKSettingsModuleCommandEditor::loadSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group ("Command Editor Windows");
	completion_enabled = cg.readEntry ("Completion enabled", true);
	completion_min_chars = cg.readEntry ("Completion min chars", 2);
	completion_timeout = cg.readEntry ("Completion timeout", 500);
	arghinting_enabled = cg.readEntry ("Argument hinting enabled", true);

	autosave_enabled = cg.readEntry ("Autosave enabled", true);
	autosave_keep = cg.readEntry ("Autosave keep saves", false);
	autosave_interval = cg.readEntry ("Autosave interval", 5);

	num_recent_files = cg.readEntry ("Max number of recent files", 10);
	script_file_filter = cg.readEntry ("Script file filter", "*.R *.S *.q *.Rhistory");
}

// static
bool RKSettingsModuleCommandEditor::matchesScriptFileFilter (const QString &filename) {
	RK_TRACE (SETTINGS);

	const QStringList exts = script_file_filter.split (' ');
	foreach (const QString ext, exts) {
		QRegExp reg (ext, Qt::CaseInsensitive, QRegExp::Wildcard);
		if (reg.exactMatch (filename)) return true;
	}
	return false;
}

#include "rksettingsmodulecommandeditor.moc"
