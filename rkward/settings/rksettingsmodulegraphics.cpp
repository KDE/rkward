/*
rksettingsmodulegraphics - This file is part of RKWard (https://rkward.kde.org). Created: Mon Sep 13 2010
SPDX-FileCopyrightText: 2010-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rksettingsmodulegraphics.h"

#include <KLocalizedString>
#include <kconfig.h>
#include <kconfiggroup.h>

#include <qlayout.h>
#include <qlabel.h>
#include <QGroupBox>
#include <qcheckbox.h>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QLineEdit>
#include <QRadioButton>
#include <QSpinBox>

#include "../misc/rkstyle.h"
#include "../rbackend/rkrinterface.h"
#include "../misc/rkspinbox.h"
#include "../misc/rkcommonfunctions.h"
#include "../misc/rkstandardicons.h"
#include "../core/robject.h"
#include "../debug.h"

// static members
RKConfigValue<double> RKSettingsModuleGraphics::graphics_width {"graphics_width", 7.0};
RKConfigValue<double> RKSettingsModuleGraphics::graphics_height {"graphics_height", 7.0};
RKConfigValue<bool> RKSettingsModuleGraphics::graphics_hist_enable {"graphics_hist_enable", true};
RKConfigValue<int> RKSettingsModuleGraphics::graphics_hist_max_length {"graphics_hist_max_length", 20};
RKConfigValue<int> RKSettingsModuleGraphics::graphics_hist_max_plotsize {"graphics_hist_max_plotsize", 4096};
RKConfigValue<bool> RKSettingsModuleGraphics::options_kde_printing {"kde printing", true};
RKConfigValue<RKSettingsModuleGraphics::DefaultDevice, int> RKSettingsModuleGraphics::default_device {"default_device", RKDevice};
RKConfigValue<QString> RKSettingsModuleGraphics::default_device_other {"default_device_custom", QString("Cairo")};
RKConfigValue<RKSettingsModuleGraphics::StandardDevicesMode, int> RKSettingsModuleGraphics::replace_standard_devices {"replace_device", ReplaceDevice};

RKSettingsModuleGraphics::RKSettingsModuleGraphics (RKSettings *gui, QWidget *parent) : RKSettingsModule(gui, parent) {
	RK_TRACE (SETTINGS);

	QVBoxLayout *main_vbox = new QVBoxLayout (this);

	QHBoxLayout *h_layout1 = new QHBoxLayout();
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
	                                  "<p>You can also specify the name of a function such as <i>Cairo</i>.</p>"), group);
	connect (default_device_group, &QButtonGroup::idClicked, this, &RKSettingsModuleGraphics::boxChanged);
	connect (default_device_other_edit, &QLineEdit::textChanged, this, &RKSettingsModuleGraphics::boxChanged);
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
#ifdef Q_OS_MACOS
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
	connect (replace_standard_devices_group, &QButtonGroup::idClicked, this, &RKSettingsModuleGraphics::boxChanged);
	h_layout1->addWidget (group);

	group = new QGroupBox(i18n("Default window size (for RK(), or embedded device windows)"));
	group_layout = new QVBoxLayout(group);
	group_layout->addWidget(new QLabel(i18n("Default width (inches):")));
	group_layout->addWidget(graphics_width.makeSpinBox(1, 100.0, this));
	group_layout->addSpacing(2*RKStyle::spacingHint());
	group_layout->addWidget(new QLabel(i18n("Default height (inches)")));
	group_layout->addWidget(graphics_height.makeSpinBox(1, 100.0, this));
	main_vbox->addWidget (group);

	main_vbox->addWidget(options_kde_printing.makeCheckbox(i18n("Use KDE printer dialog for printing devices (if available)"), this));

	graphics_hist_box = new QGroupBox (i18n ("Screen device history"), this);
	graphics_hist_box->setCheckable (true);
	graphics_hist_box->setChecked (graphics_hist_enable);
	group_layout = new QVBoxLayout (graphics_hist_box);
	connect (graphics_hist_box, &QGroupBox::toggled, this, &RKSettingsModuleGraphics::boxChanged);
	h_layout = new QHBoxLayout ();
	group_layout->addLayout (h_layout);
	h_layout->addWidget (new QLabel (i18n ("Maximum number of recorded plots:"), graphics_hist_box));
	h_layout->addWidget (graphics_hist_max_length.makeSpinBox(1, 200, this));
	h_layout = new QHBoxLayout ();
	group_layout->addLayout (h_layout);
	h_layout->addWidget (new QLabel (i18n ("Maximum size of a single recorded plot (in KB):"), graphics_hist_box));
	h_layout->addWidget (graphics_hist_max_plotsize.makeSpinBox(4, 50000, this));
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

QString RKSettingsModuleGraphics::caption() const {
	RK_TRACE(SETTINGS);
	return(i18n("Onscreen Graphics"));
}

QIcon RKSettingsModuleGraphics::icon() const {
	RK_TRACE(SETTINGS);
	return RKStandardIcons::getIcon(RKStandardIcons::WindowX11);
}

void RKSettingsModuleGraphics::applyChanges () {
	RK_TRACE (SETTINGS);

	default_device = (DefaultDevice) default_device_group->checkedId ();
	default_device_other = default_device_other_edit->text ();
	replace_standard_devices = (StandardDevicesMode) replace_standard_devices_group->checkedId ();

	graphics_hist_enable = graphics_hist_box->isChecked ();

	QStringList commands = makeRRunTimeOptionCommands ();
	for (QStringList::const_iterator it = commands.cbegin (); it != commands.cend (); ++it) {
		RInterface::issueCommand(new RCommand(*it, RCommand::App), commandChain ());
	}
}

void RKSettingsModuleGraphics::syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction a) {
	RK_TRACE(SETTINGS);

	KConfigGroup cg = config->group("Graphics Device Windows");
	default_device.syncConfig(cg, a);
	default_device_other.syncConfig(cg, a);
	replace_standard_devices.syncConfig(cg, a);
	graphics_width.syncConfig(cg, a);
	graphics_height.syncConfig(cg, a);
	graphics_hist_enable.syncConfig(cg, a);
	graphics_hist_max_length.syncConfig(cg, a);
	graphics_hist_max_plotsize.syncConfig(cg, a);
	options_kde_printing.syncConfig(cg, a);
}

//static
QStringList RKSettingsModuleGraphics::makeRRunTimeOptionCommands () {
	RK_TRACE (SETTINGS);
	QStringList list;

	// register RK as interactive
	list.append ("try (if (!(\"RKGraphicsDevice\" %in% deviceIsInteractive())) deviceIsInteractive(name=\"RKGraphicsDevice\"))\n");

	QString command = "options (device=";
	if (default_device == RKDevice) command.append ("\"RK\"");
	else if (default_device == OtherDevice) command.append (RObject::rQuote (default_device_other));
	else {
#if defined Q_OS_WIN
	command.append ("\"windows\"");
#elif defined Q_OS_MACOS
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

