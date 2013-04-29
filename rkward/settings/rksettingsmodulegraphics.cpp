/***************************************************************************
                          rksettingsmodulegraphics  -  description
                             -------------------
    begin                : Mon Sep 13 2010
    copyright            : (C) 2010, 2013 by Thomas Friedrichsmeier
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
#include <QButtonGroup>
#include <QLineEdit>
#include <QRadioButton>

#include "../rkglobals.h"
#include "../rbackend/rinterface.h"
#include "../misc/rkspinbox.h"
#include "../misc/rkcommonfunctions.h"
#include "../debug.h"
#include "../core/robject.h"

// static members
double RKSettingsModuleGraphics::graphics_width;
double RKSettingsModuleGraphics::graphics_height;
bool RKSettingsModuleGraphics::graphics_hist_enable;
int RKSettingsModuleGraphics::graphics_hist_max_length;
int RKSettingsModuleGraphics::graphics_hist_max_plotsize;
bool RKSettingsModuleGraphics::options_kde_printing;
RKSettingsModuleGraphics::DefaultDevice RKSettingsModuleGraphics::default_device;
QString RKSettingsModuleGraphics::default_device_other;
RKSettingsModuleGraphics::StandardDevicesMode RKSettingsModuleGraphics::replace_standard_devices;

RKSettingsModuleGraphics::RKSettingsModuleGraphics (RKSettings *gui, QWidget *parent) : RKSettingsModule(gui, parent) {
	RK_TRACE (SETTINGS);

	QVBoxLayout *main_vbox = new QVBoxLayout (this);

	QHBoxLayout *h_layout1 = new QHBoxLayout (this);
	main_vbox->addLayout (h_layout1);
	QGroupBox *group = new QGroupBox (i18n ("Default graphics device"), this);
	default_device_group = new QButtonGroup (this);
	QVBoxLayout* group_layout = new QVBoxLayout (group);
	QRadioButton *button = new QRadioButton (i18n ("RKWard native device"), group);
	default_device_group->addButton (button, (int) RKDevice);
	group_layout->addWidget (button);
	button = new QRadioButton (i18n ("Platform default device"), group);
	default_device_group->addButton (button, (int) PlatformDevice);
	group_layout->addWidget (button);
	button = new QRadioButton (i18n ("Other device:"), group);
	default_device_group->addButton (button, (int) OtherDevice);
	QHBoxLayout *h_layout = new QHBoxLayout ();
	group_layout->addLayout (h_layout);
	h_layout->addWidget (button);
	default_device_other_edit = new QLineEdit (default_device_other, group);
	h_layout->addWidget (default_device_other_edit);
	button = static_cast<QRadioButton*> (default_device_group->button ((int) default_device));
	if (button) button->setChecked (true);
	RKCommonFunctions::setTips (i18n ("<p>The default device to be used for plotting, i.e. when new plot is created, while no graphics device is active (see <i>options(\"device\")</i>).</p>"
	                                  "<p>The RKWard native device is the recommended choice for most users. This corresponds to the R command <i>RK()</i>.</p>"
	                                  "<p>The 'Platform default device' corresponds to one of <i>X11()</i>, <i>windows()</i>, or <i>quartz()</i>, depending on the platform.</p>"
	                                  "<p>You can also specify the name of a function such as <i>cairoDevice</i>.</p>"), group);
	connect (default_device_group, SIGNAL (buttonClicked(int)), this, SLOT (boxChanged()));
	connect (default_device_other_edit, SIGNAL (textChanged(const QString&)), this, SLOT (boxChanged()));
	h_layout1->addWidget (group);

	group = new QGroupBox (i18n ("Integration of R standard devices"), this);
	replace_standard_devices_group = new QButtonGroup (this);
	group_layout = new QVBoxLayout (group);
	button = new QRadioButton (i18n ("Replace with RKWard device"), group);
	replace_standard_devices_group->addButton (button, (int) ReplaceDevice);
	group_layout->addWidget (button);
	button = new QRadioButton (i18n ("Embed original device"), group);
	replace_standard_devices_group->addButton (button, (int) EmbedDevice);
	group_layout->addWidget (button);
#if not (defined Q_WS_X11 || defined Q_WS_WIN)
	button->setEnabled (false);
#endif
	button = new QRadioButton (i18n ("No device integration"), group);
	replace_standard_devices_group->addButton (button, (int) LeaveDevice);
	group_layout->addWidget (button);
	button = static_cast<QRadioButton*> (replace_standard_devices_group->button ((int) replace_standard_devices));
	if (button) button->setChecked (true);
	RKCommonFunctions::setTips (i18n ("<p>Many scripts use calls to platform specific standard devices (<i>X11()</i>, <i>windows()</i>, <i>quartz()</i>), although any on-screen device "
	                                  "could be used at these places. RKWard provides overloads for these standard device functions, which can change their behavior when used in "
	                                  "user code:</p>"
	                                  "<ul><li>The calls can be re-directed to the RKWard native device (<i>RK()</i>). Some, but not all function arguments will be matched, others will "
	                                  "be ignored.</li>"
	                                  "<li>The original platform specific devices can be used, but embedded into RKWard windows. This option is not available on MacOS X.</li>"
	                                  "<li>The original platform specific devices can be used unchanged, without the addition of RKWard specific features.</li></ul>"
	                                  "<p>Regardless of this setting, the original devices are always accessible as <i>grDevices::X11()</i>, etc.</p>"
	                                  "<p>Using a device on a platform where it is not defined (e.g. <i>Windows()</i> on Mac OS X) will always fall back to the <i>RK()</i> device.</p>"), group);
	connect (replace_standard_devices_group, SIGNAL (buttonClicked(int)), this, SLOT (boxChanged()));
	h_layout1->addWidget (group);

	group = new QGroupBox (i18n ("Default window size (for RK(), or embedded device windows)"), this);
	group_layout = new QVBoxLayout (group);
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
	h_layout = new QHBoxLayout ();
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
	updateControls ();
}

RKSettingsModuleGraphics::~RKSettingsModuleGraphics() {
	RK_TRACE (SETTINGS);
}

void RKSettingsModuleGraphics::updateControls () {
	RK_TRACE (SETTINGS);
	default_device_other_edit->setEnabled (default_device_group->checkedId () == (int) OtherDevice);
	QRadioButton *button = static_cast<QRadioButton*> (replace_standard_devices_group->button ((int) ReplaceDevice));
	if (button) button->setEnabled (default_device_group->checkedId () != PlatformDevice);
}

void RKSettingsModuleGraphics::boxChanged () {
	RK_TRACE (SETTINGS);

	updateControls ();
	change ();
}

QString RKSettingsModuleGraphics::caption () {
	RK_TRACE (SETTINGS);
	return (i18n ("Onscreen Graphics"));
}

void RKSettingsModuleGraphics::applyChanges () {
	RK_TRACE (SETTINGS);

	default_device = (DefaultDevice) default_device_group->checkedId ();
	default_device_other = default_device_other_edit->text ();
	replace_standard_devices = (StandardDevicesMode) replace_standard_devices_group->checkedId ();
	
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
	cg.writeEntry ("default_device", (int) default_device);
	cg.writeEntry ("default_device_custom", default_device_other);
	cg.writeEntry ("replace_device", (int) replace_standard_devices);
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
	default_device = (DefaultDevice) cg.readEntry ("default_device", (int) RKDevice);
	default_device_other = cg.readEntry ("default_device_custom", QString ("cairoDevice"));
	replace_standard_devices = (StandardDevicesMode) cg.readEntry ("replace_device", (int) ReplaceDevice);
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

	// register RK as interactive
	list.append ("try (if (!(\"RK\" %in% deviceIsInteractive())) deviceIsInteractive(name=\"RK\"))\n");

	QString command = "options (device=";
	if (default_device == RKDevice) command.append ("\"RK\"");
	else if (default_device == OtherDevice) command.append (RObject::rQuote (default_device_other));
	else {
#if defined Q_WS_WIN
	command.append ("\"windows\"");
#elif defined Q_OS_MAC
	command.append ("ifelse (capabilities(\"quartz\"), \"quartz\", \"X11\")");
#else
	command.append ("\"X11\"");
#endif
	}
	list.append (command + ")\n");

	command = "options (rk.override.platform.devices=\"";
	if ((replace_standard_devices == ReplaceDevice) && (default_device != PlatformDevice)) {
		command.append ("replace");
	} else if (replace_standard_devices == LeaveDevice) {
		command.append ("nointegration");
	} else {
		command.append ("embed");
	}
	list.append (command + "\")\n");

	command = "options";
	command.append ("(\"rk.screendevice.width\"=" + QString::number (graphics_width));
	command.append (", \"rk.screendevice.height\"=" + QString::number (graphics_height) + ")\n");
	list.append (command);
	list.append ("options (\"rk.graphics.hist.max.plotsize\"=" + QString::number (graphics_hist_max_plotsize) + ")\n");
	list.append ("rk.toggle.plot.history (" + QString (graphics_hist_enable?"TRUE":"FALSE") + ")\n");
	list.append ("rk.verify.plot.hist.limits (" + QString::number (graphics_hist_max_length) + ")\n");
	
	return (list);
}

#include "rksettingsmodulegraphics.moc"
