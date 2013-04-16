/***************************************************************************
                          rksettingsmodulegraphics  -  description
                             -------------------
    begin                : Mon Sep 13 2010
    copyright            : (C) 2010 by Thomas Friedrichsmeier
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
#include "rksettingsmodulegraphics.h"

#include <klocale.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <knuminput.h>

#include <qlayout.h>
#include <qlabel.h>
#include <QGroupBox>
#include <qcheckbox.h>
#include <QVBoxLayout>

#include "../rkglobals.h"
#include "../rbackend/rinterface.h"
#include "../misc/rkspinbox.h"
#include "../debug.h"

// static members
double RKSettingsModuleGraphics::graphics_width;
double RKSettingsModuleGraphics::graphics_height;
bool RKSettingsModuleGraphics::graphics_hist_enable;
int RKSettingsModuleGraphics::graphics_hist_max_length;
int RKSettingsModuleGraphics::graphics_hist_max_plotsize;
bool RKSettingsModuleGraphics::options_kde_printing;

RKSettingsModuleGraphics::RKSettingsModuleGraphics (RKSettings *gui, QWidget *parent) : RKSettingsModule(gui, parent) {
	RK_TRACE (SETTINGS);

	QVBoxLayout *main_vbox = new QVBoxLayout (this);
	
	QGroupBox *group = new QGroupBox (i18n ("Default window size"), this);
	QVBoxLayout* group_layout = new QVBoxLayout (group);
	group_layout->addWidget (new QLabel (i18n ("Default width (inches):"), group));
	group_layout->addWidget (graphics_width_box = new RKSpinBox (group));
	graphics_width_box->setRealMode (1, 100.0, graphics_width, 1, 3);
	group_layout->addSpacing (2*RKGlobals::spacingHint ());
	group_layout->addWidget (new QLabel (i18n ("Default height (inches)"), group));
	group_layout->addWidget (graphics_height_box = new RKSpinBox (group));
	graphics_height_box->setRealMode (1, 100.0, graphics_height, 1, 3);
	connect (graphics_width_box, SIGNAL (valueChanged (int)), this, SLOT (boxChanged ()));
	connect (graphics_height_box, SIGNAL (valueChanged (int)), this, SLOT (boxChanged ()));
	main_vbox->addWidget (group);

	kde_printing_box = new QCheckBox (i18n ("Use KDE printer dialog for printing devices (if available)"), this);
	kde_printing_box->setChecked (options_kde_printing);
	connect (kde_printing_box, SIGNAL (stateChanged(int)), this, SLOT (boxChanged()));
	main_vbox->addWidget (kde_printing_box);

	graphics_hist_box = new QGroupBox (i18n ("Screen device history"), this);
	graphics_hist_box->setCheckable (true);
	graphics_hist_box->setChecked (graphics_hist_enable);
	group_layout = new QVBoxLayout (graphics_hist_box);
	connect (graphics_hist_box, SIGNAL (toggled (bool)), this, SLOT (boxChanged ()));
	QHBoxLayout *h_layout = new QHBoxLayout ();
	group_layout->addLayout (h_layout);
	h_layout->addWidget (new QLabel (i18n ("Maximum number of recorded plots:"), graphics_hist_box));
	h_layout->addWidget (graphics_hist_max_length_box = new KIntSpinBox (1, 200, 1, graphics_hist_max_length, graphics_hist_box));
	h_layout = new QHBoxLayout ();
	group_layout->addLayout (h_layout);
	h_layout->addWidget (new QLabel (i18n ("Maximum size of a single recorded plot (in KB):"), graphics_hist_box));
	h_layout->addWidget (graphics_hist_max_plotsize_box = new KIntSpinBox (4, 20000, 4, graphics_hist_max_plotsize, graphics_hist_box));
	connect (graphics_hist_max_length_box, SIGNAL (valueChanged (int)), this, SLOT (boxChanged ()));
	connect (graphics_hist_max_plotsize_box, SIGNAL (valueChanged (int)), this, SLOT (boxChanged ()));

	main_vbox->addWidget (graphics_hist_box);

	main_vbox->addStretch ();
}

RKSettingsModuleGraphics::~RKSettingsModuleGraphics() {
	RK_TRACE (SETTINGS);
}

void RKSettingsModuleGraphics::boxChanged () {
	RK_TRACE (SETTINGS);
	change ();
}

QString RKSettingsModuleGraphics::caption () {
	RK_TRACE (SETTINGS);
	return (i18n ("Onscreen Graphics"));
}

void RKSettingsModuleGraphics::applyChanges () {
	RK_TRACE (SETTINGS);

	graphics_width = graphics_width_box->realValue ();
	graphics_height = graphics_height_box->realValue ();

	graphics_hist_enable = graphics_hist_box->isChecked ();
	graphics_hist_max_length = graphics_hist_max_length_box->value ();
	graphics_hist_max_plotsize = graphics_hist_max_plotsize_box->value ();

	options_kde_printing = kde_printing_box->isChecked ();

	QStringList commands = makeRRunTimeOptionCommands ();
	for (QStringList::const_iterator it = commands.begin (); it != commands.end (); ++it) {
		RKGlobals::rInterface ()->issueCommand (*it, RCommand::App, QString::null, 0, 0, commandChain ());
	}
}

void RKSettingsModuleGraphics::save (KConfig *config) {
	RK_TRACE (SETTINGS);

	saveSettings (config);
}

void RKSettingsModuleGraphics::saveSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group ("Graphics Device Windows");
	cg.writeEntry ("graphics_width", graphics_width);
	cg.writeEntry ("graphics_height", graphics_height);
	cg.writeEntry ("graphics_hist_enable", graphics_hist_enable);
	cg.writeEntry ("graphics_hist_max_length", graphics_hist_max_length);
	cg.writeEntry ("graphics_hist_max_plotsize", graphics_hist_max_plotsize);
	cg.writeEntry ("kde printing", options_kde_printing);
}

void RKSettingsModuleGraphics::loadSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group ("Graphics Device Windows");
	graphics_width = cg.readEntry ("graphics_width", 7.0);
	graphics_height = cg.readEntry ("graphics_height", 7.0);
	graphics_hist_enable = cg.readEntry ("graphics_hist_enable", true);
	graphics_hist_max_length = cg.readEntry ("graphics_hist_max_length", 20);
	graphics_hist_max_plotsize = cg.readEntry ("graphics_hist_max_plotsize", 1024);
	options_kde_printing = cg.readEntry ("kde printing", true);
}

//static
QStringList RKSettingsModuleGraphics::makeRRunTimeOptionCommands () {
	RK_TRACE (SETTINGS);
	QStringList list;

#ifdef Q_WS_X11
	QString command = "X11.options";
	command.append ("(\"width\"=" + QString::number (graphics_width));
	command.append (", \"height\"=" + QString::number (graphics_height) + ")\n");
#else
	QString command = "options";
	command.append ("(\"rk.screendevice.width\"=" + QString::number (graphics_width));
	command.append (", \"rk.screendevice.height\"=" + QString::number (graphics_height) + ")\n");
#endif
	list.append (command);
	list.append ("options (\"rk.graphics.hist.max.plotsize\"=" + QString::number (graphics_hist_max_plotsize) + ")\n");
	list.append ("rk.toggle.plot.history (" + QString (graphics_hist_enable?"TRUE":"FALSE") + ")\n");
	list.append ("rk.verify.plot.hist.limits (" + QString::number (graphics_hist_max_length) + ")\n");
	
	return (list);
}

#include "rksettingsmodulegraphics.moc"
