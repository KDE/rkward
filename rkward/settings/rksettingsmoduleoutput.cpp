/***************************************************************************
                          rksettingsmoduleoutput  -  description
                             -------------------
    begin                : Fri Jul 30 2004
    copyright            : (C) 2004, 2010 by Thomas Friedrichsmeier
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
#include "rksettingsmoduleoutput.h"

#include <klocale.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <knuminput.h>

#include <qlayout.h>
#include <qlabel.h>
#include <QGroupBox>
#include <qcheckbox.h>
#include <QVBoxLayout>
#include <QComboBox>

#include "../rkglobals.h"
#include "../rbackend/rinterface.h"
#include "../debug.h"

// static members
bool RKSettingsModuleOutput::auto_show;
bool RKSettingsModuleOutput::auto_raise;
QString RKSettingsModuleOutput::graphics_type;
int RKSettingsModuleOutput::graphics_width;
int RKSettingsModuleOutput::graphics_height;
int RKSettingsModuleOutput::graphics_jpg_quality;
bool RKSettingsModuleOutput::graphics_hist_enable;
int RKSettingsModuleOutput::graphics_hist_max_length;
int RKSettingsModuleOutput::graphics_hist_max_plotsize;

RKSettingsModuleOutput::RKSettingsModuleOutput (RKSettings *gui, QWidget *parent) : RKSettingsModule(gui, parent) {
	RK_TRACE (SETTINGS);

	QVBoxLayout *main_vbox = new QVBoxLayout (this);
	
	QGroupBox *group = new QGroupBox (i18n ("Output Window options"), this);
	QVBoxLayout* group_layout = new QVBoxLayout (group);
	group_layout->addWidget (auto_show_box = new QCheckBox (i18n ("show window on new output"), group));
	auto_show_box->setChecked (auto_show);
	connect (auto_show_box, SIGNAL (stateChanged (int)), this, SLOT (boxChanged (int)));
	group_layout->addWidget (auto_raise_box = new QCheckBox (i18n ("raise window on new output"), group));
	auto_raise_box->setChecked (auto_raise);
	auto_raise_box->setEnabled (auto_show);
	connect (auto_raise_box, SIGNAL (stateChanged (int)), this, SLOT (boxChanged (int)));

	main_vbox->addWidget (group);

	group = new QGroupBox (i18n ("Graphics"), this);
	group_layout = new QVBoxLayout (group);
	QHBoxLayout *h_layout = new QHBoxLayout (group);
	group_layout->addLayout (h_layout);
	h_layout->addWidget (new QLabel (i18n ("File format"), group));
	h_layout->addWidget (graphics_type_box = new QComboBox (group));
	graphics_type_box->addItem (i18n ("<Default>"), QString ("NULL"));
	graphics_type_box->addItem (i18n ("PNG"), QString ("\"PNG\""));
	graphics_type_box->addItem (i18n ("SVG"), QString ("\"SVG\""));
	graphics_type_box->addItem (i18n ("JPG"), QString ("\"JPG\""));
	graphics_type_box->setCurrentIndex (graphics_type_box->findData (graphics_type));
	graphics_type_box->setEditable (false);
	connect (graphics_type_box, SIGNAL (currentIndexChanged (int)), this, SLOT (boxChanged (int)));
	h_layout->addSpacing (2*RKGlobals::spacingHint ());
	h_layout->addWidget (new QLabel (i18n ("JPG quality"), group));
	h_layout->addWidget (graphics_jpg_quality_box = new KIntSpinBox (1, 100, 1, graphics_jpg_quality, group));
	graphics_jpg_quality_box->setEnabled (graphics_type == "\"JPG\"");
	connect (graphics_jpg_quality_box, SIGNAL (valueChanged (int)), this, SLOT (boxChanged (int)));
	h_layout->addStretch ();

	h_layout = new QHBoxLayout (group);
	group_layout->addLayout (h_layout);
	h_layout->addWidget (new QLabel (i18n ("Width:"), group));
	h_layout->addWidget (graphics_width_box = new KIntSpinBox (1, INT_MAX, 1, graphics_width, group));
	h_layout->addSpacing (2*RKGlobals::spacingHint ());
	h_layout->addWidget (new QLabel (i18n ("Height:"), group));
	h_layout->addWidget (graphics_height_box = new KIntSpinBox (1, INT_MAX, 1, graphics_height, group));
	h_layout->addStretch ();
	connect (graphics_width_box, SIGNAL (valueChanged (int)), this, SLOT (boxChanged (int)));
	connect (graphics_height_box, SIGNAL (valueChanged (int)), this, SLOT (boxChanged (int)));

	main_vbox->addWidget (group);

	group = new QGroupBox (i18n ("Screen device history"), this);
	group_layout = new QVBoxLayout (group);
	group_layout->addWidget (graphics_hist_enable_box = new QCheckBox (i18n ("enable screen device history"), group));
	graphics_hist_enable_box->setChecked (graphics_hist_enable);
	connect (graphics_hist_enable_box, SIGNAL (stateChanged (int)), this, SLOT (boxChanged (int)));
	h_layout = new QHBoxLayout (group);
	group_layout->addLayout (h_layout);
	h_layout->addWidget (new QLabel (i18n ("Maximum number of recorded plots:"), group));
	h_layout->addWidget (graphics_hist_max_length_box = new KIntSpinBox (1, 200, 1, graphics_hist_max_length, group));
	graphics_hist_max_length_box->setEnabled (graphics_hist_enable);
	h_layout = new QHBoxLayout (group);
	group_layout->addLayout (h_layout);
	h_layout->addWidget (new QLabel (i18n ("Maximum size of a single recorded plot (in KB):"), group));
	h_layout->addWidget (graphics_hist_max_plotsize_box = new KIntSpinBox (4, 20000, 4, graphics_hist_max_plotsize, group)); // in KB
	graphics_hist_max_plotsize_box->setEnabled (graphics_hist_enable);
	connect (graphics_hist_max_length_box, SIGNAL (valueChanged (int)), this, SLOT (boxChanged (int)));
	connect (graphics_hist_max_plotsize_box, SIGNAL (valueChanged (int)), this, SLOT (boxChanged (int)));

	main_vbox->addWidget (group);

	main_vbox->addStretch ();
}

RKSettingsModuleOutput::~RKSettingsModuleOutput() {
	RK_TRACE (SETTINGS);
}

void RKSettingsModuleOutput::boxChanged (int) {
	RK_TRACE (SETTINGS);
	change ();
	auto_raise_box->setEnabled (auto_show_box->isChecked ());
	graphics_hist_max_length_box->setEnabled (graphics_hist_enable_box->isChecked ());
	graphics_hist_max_plotsize_box->setEnabled (graphics_hist_enable_box->isChecked ());
	graphics_jpg_quality_box->setEnabled (graphics_type_box->itemData (graphics_type_box->currentIndex ()).toString () == "\"JPG\"");
}

QString RKSettingsModuleOutput::caption () {
	RK_TRACE (SETTINGS);
	return (i18n ("Output"));
}

bool RKSettingsModuleOutput::hasChanges () {
	RK_TRACE (SETTINGS);
	return changed;
}

void RKSettingsModuleOutput::applyChanges () {
	RK_TRACE (SETTINGS);

	auto_show = auto_show_box->isChecked ();
	auto_raise = auto_raise_box->isChecked ();

	graphics_type = graphics_type_box->itemData (graphics_type_box->currentIndex ()).toString ();
	graphics_width = graphics_width_box->value ();
	graphics_height = graphics_height_box->value ();
	graphics_jpg_quality = graphics_jpg_quality_box->value ();

	graphics_hist_enable = graphics_hist_enable_box->isChecked ();
	graphics_hist_max_length = graphics_hist_max_length_box->value ();
	graphics_hist_max_plotsize = graphics_hist_max_plotsize_box->value ();

	QStringList commands = makeRRunTimeOptionCommands ();
	for (QStringList::const_iterator it = commands.begin (); it != commands.end (); ++it) {
		RKGlobals::rInterface ()->issueCommand (*it, RCommand::App, QString::null, 0, 0, commandChain ());
	}
}

void RKSettingsModuleOutput::save (KConfig *config) {
	RK_TRACE (SETTINGS);

	saveSettings (config);
}

void RKSettingsModuleOutput::saveSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group ("Output Window");
	cg.writeEntry ("auto_show", auto_show);
	cg.writeEntry ("auto_raise", auto_raise);
	cg.writeEntry ("graphics_type", graphics_type);
	cg.writeEntry ("graphics_width", graphics_width);
	cg.writeEntry ("graphics_height", graphics_height);
	cg.writeEntry ("graphics_jpg_quality", graphics_jpg_quality);
	cg.writeEntry ("graphics_hist_enable", graphics_hist_enable);
	cg.writeEntry ("graphics_hist_max_length", graphics_hist_max_length);
	cg.writeEntry ("graphics_hist_max_plotsize", graphics_hist_max_plotsize);
}

void RKSettingsModuleOutput::loadSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group ("Output Window");
	auto_show = cg.readEntry ("auto_show", true);
	auto_raise = cg.readEntry ("auto_raise", true);
	graphics_type = cg.readEntry ("graphics_type", "NULL");
	graphics_width = cg.readEntry ("graphics_width", 480);
	graphics_height = cg.readEntry ("graphics_height", 480);
	graphics_jpg_quality = cg.readEntry ("graphics_jpg_quality", 75);
	graphics_hist_enable = cg.readEntry ("graphics_hist_enable", true);
	graphics_hist_max_length = cg.readEntry ("graphics_hist_max_length", 20);
	graphics_hist_max_plotsize = cg.readEntry ("graphics_hist_max_plotsize", 1024);
}

//static
QStringList RKSettingsModuleOutput::makeRRunTimeOptionCommands () {
	QStringList list;

// output format options
	QString command = "options (\"rk.graphics.type\"=" + graphics_type;
	command.append (", \"rk.graphics.width\"=" + QString::number (graphics_width));
	command.append (", \"rk.graphics.height\"=" + QString::number (graphics_height));
	if (graphics_type == "\"JPG\"") command.append (", \"rk.graphics.jpg.quality\"=" + QString::number (graphics_jpg_quality));
	//command.append (", \"rk.graphics.hist.max.length\"=" + QString::number (graphics_hist_max_length));
	command.append (", \"rk.graphics.hist.max.plotsize\"=" + QString::number (graphics_hist_max_plotsize));
	command.append (")\nrk.toggle.plot.history (" + QString (graphics_hist_enable?"TRUE":"FALSE") + ")\n");
	list.append (command + "rk.verify.plot.hist.limits (" + QString::number (graphics_hist_max_length) + ")\n");
	
	return (list);
}

#include "rksettingsmoduleoutput.moc"
