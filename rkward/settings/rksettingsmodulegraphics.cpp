/*
rksettingsmodulegraphics - This file is part of RKWard (https://rkward.kde.org). Created: Mon Sep 13 2010
SPDX-FileCopyrightText: 2010-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rksettingsmodulegraphics.h"

#include <KLocalizedString>
#include <kconfig.h>
#include <kconfiggroup.h>

#include <QLineEdit>
#include <QSpinBox>
#include <QVBoxLayout>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>

#include "../core/robject.h"
#include "../debug.h"
#include "../misc/rkcommonfunctions.h"
#include "../misc/rkradiogroup.h"
#include "../misc/rkspinbox.h"
#include "../misc/rkstandardicons.h"
#include "../misc/rkstyle.h"
#include "../rbackend/rkrinterface.h"
#include "rksettings.h"

// static members
RKConfigValue<double> RKSettingsModuleGraphics::graphics_width{"graphics_width", 7.0};
RKConfigValue<double> RKSettingsModuleGraphics::graphics_height{"graphics_height", 7.0};
RKConfigValue<bool> RKSettingsModuleGraphics::graphics_hist_enable{"graphics_hist_enable", true};
RKConfigValue<int> RKSettingsModuleGraphics::graphics_hist_max_length{"graphics_hist_max_length", 20};
RKConfigValue<int> RKSettingsModuleGraphics::graphics_hist_max_plotsize{"graphics_hist_max_plotsize", 4096};
RKConfigValue<bool> RKSettingsModuleGraphics::options_kde_printing{"kde printing", true};
RKConfigValue<RKSettingsModuleGraphics::DefaultDevice, int> RKSettingsModuleGraphics::default_device{"default_device", RKDevice};
RKConfigValue<QString> RKSettingsModuleGraphics::default_device_other{"default_device_custom", QStringLiteral("Cairo")};
RKConfigValue<RKSettingsModuleGraphics::StandardDevicesMode, int> RKSettingsModuleGraphics::replace_standard_devices{"replace_device", ReplaceDevice};

class RKSettingsPageGraphics : public RKSettingsModuleWidget {
  public:
	RKSettingsPageGraphics(QWidget *parent, RKSettingsModule *parent_module) : RKSettingsModuleWidget(parent, parent_module, RKSettingsModuleGraphics::page_id) {
		RK_TRACE(SETTINGS);

		setWindowTitle(i18n("Onscreen Graphics"));
		setWindowIcon(RKStandardIcons::getIcon(RKStandardIcons::WindowX11));
		help_url = QUrl(QStringLiteral("rkward://page/rkward_plot_history#scd_settings"));

		QVBoxLayout *main_vbox = new QVBoxLayout(this);

		QHBoxLayout *h_layout1 = new QHBoxLayout();
		main_vbox->addLayout(h_layout1);
		auto group = new RKRadioGroup(i18n("Default graphics device"));
		default_device_group = group->group();
		group->addButton(i18n("RKWard native device"), (int)RKSettingsModuleGraphics::RKDevice);
		group->addButton(i18n("Platform default device"), (int)RKSettingsModuleGraphics::PlatformDevice);
		auto default_device_other_edit = RKSettingsModuleGraphics::default_device_other.makeLineEdit(this);
		RKCommonFunctions::setTips(i18n("<p>The default device to be used for plotting, i.e. when new plot is created, while no graphics device is active (see <i>options(\"device\")</i>).</p>"
		                                "<p>The RKWard native device is the recommended choice for most users. This corresponds to the R command <i>RK()</i>.</p>"
		                                "<p>The 'Platform default device' corresponds to one of <i>X11()</i>, <i>windows()</i>, or <i>quartz()</i>, depending on the platform.</p>"
		                                "<p>You can also specify the name of a function such as <i>Cairo</i>.</p>"),
		                           group);
		group->addButton(i18n("Other device:"), (int)RKSettingsModuleGraphics::OtherDevice, default_device_other_edit, QBoxLayout::LeftToRight);
		connect(default_device_group, &QButtonGroup::idClicked, this, &RKSettingsPageGraphics::boxChanged);
		group->setButtonChecked((int)RKSettingsModuleGraphics::default_device, true);
		h_layout1->addWidget(group);

		group = new RKRadioGroup(i18n("Integration of R standard devices"));
		replace_standard_devices_group = group->group();
		group->addButton(i18n("Replace with RKWard device"), (int)RKSettingsModuleGraphics::ReplaceDevice);
#ifdef Q_OS_MACOS
		group->addButton(i18n("Embed original device"), (int)RKSettingsModuleGraphics::EmbedDevice)->setEnabled(false);
#else
		group->addButton(i18n("Embed original device"), (int)RKSettingsModuleGraphics::EmbedDevice);
#endif
		group->addButton(i18n("No device integration"), (int)RKSettingsModuleGraphics::LeaveDevice);
		group->setButtonChecked((int)RKSettingsModuleGraphics::replace_standard_devices, true);
		RKCommonFunctions::setTips(i18n("<p>Many scripts use calls to platform specific standard devices (<i>X11()</i>, <i>windows()</i>, <i>quartz()</i>), although any on-screen device "
		                                "could be used at these places. RKWard provides overloads for these standard device functions, which can change their behavior when used in "
		                                "user code:</p>"
		                                "<ul><li>The calls can be re-directed to the RKWard native device (<i>RK()</i>). Some, but not all function arguments will be matched, others will "
		                                "be ignored.</li>"
		                                "<li>The original platform specific devices can be used, but embedded into RKWard windows. This option is not available on MacOS X.</li>"
		                                "<li>The original platform specific devices can be used unchanged, without the addition of RKWard specific features.</li></ul>"
		                                "<p>Regardless of this setting, the original devices are always accessible as <i>grDevices::X11()</i>, etc.</p>"
		                                "<p>Using a device on a platform where it is not defined (e.g. <i>Windows()</i> on Mac OS X) will always fall back to the <i>RK()</i> device.</p>"),
		                           group);
		connect(replace_standard_devices_group, &QButtonGroup::idClicked, this, &RKSettingsPageGraphics::boxChanged);
		h_layout1->addWidget(group);

		auto sgroup = new QGroupBox(i18n("Default window size (for RK(), or embedded device windows)"));
		auto group_layout = new QVBoxLayout(sgroup);
		group_layout->addWidget(new QLabel(i18n("Default width (inches):")));
		group_layout->addWidget(RKSettingsModuleGraphics::graphics_width.makeSpinBox(1, 100.0, this));
		group_layout->addSpacing(2 * RKStyle::spacingHint());
		group_layout->addWidget(new QLabel(i18n("Default height (inches)")));
		group_layout->addWidget(RKSettingsModuleGraphics::graphics_height.makeSpinBox(1, 100.0, this));
		main_vbox->addWidget(sgroup);

		main_vbox->addWidget(RKSettingsModuleGraphics::options_kde_printing.makeCheckbox(i18n("Use KDE printer dialog for printing devices (if available)"), this));

		auto graphics_hist_box = RKSettingsModuleGraphics::graphics_hist_enable.makeCheckableGroupBox(i18n("Screen device history"), this);
		group_layout = new QVBoxLayout(graphics_hist_box);
		auto h_layout = new QHBoxLayout();
		group_layout->addLayout(h_layout);
		h_layout->addWidget(new QLabel(i18n("Maximum number of recorded plots:")));
		h_layout->addWidget(RKSettingsModuleGraphics::graphics_hist_max_length.makeSpinBox(1, 200, this));
		h_layout = new QHBoxLayout();
		group_layout->addLayout(h_layout);
		h_layout->addWidget(new QLabel(i18n("Maximum size of a single recorded plot (in KB):")));
		h_layout->addWidget(RKSettingsModuleGraphics::graphics_hist_max_plotsize.makeSpinBox(4, 50000, this));
		main_vbox->addWidget(graphics_hist_box);

		main_vbox->addStretch();
		updateControls();
	}
	~RKSettingsPageGraphics() {
		RK_TRACE(SETTINGS);
	}
	void updateControls() {
		RK_TRACE(SETTINGS);
		QRadioButton *button = static_cast<QRadioButton *>(replace_standard_devices_group->button((int)RKSettingsModuleGraphics::ReplaceDevice));
		if (button) button->setEnabled(default_device_group->checkedId() != RKSettingsModuleGraphics::PlatformDevice);
	}

	void boxChanged() {
		RK_TRACE(SETTINGS);

		updateControls();
		change();
	}

	void applyChanges() override {
		RK_TRACE(SETTINGS);

		RKSettingsModuleGraphics::default_device = (RKSettingsModuleGraphics::DefaultDevice)default_device_group->checkedId();
		RKSettingsModuleGraphics::replace_standard_devices = (RKSettingsModuleGraphics::StandardDevicesMode)replace_standard_devices_group->checkedId();

		QStringList commands = RKSettingsModuleGraphics::makeRRunTimeOptionCommands();
		for (QStringList::const_iterator it = commands.cbegin(); it != commands.cend(); ++it) {
			RInterface::issueCommand(new RCommand(*it, RCommand::App), parent_module->commandChain());
		}
	}

  private:
	QButtonGroup *default_device_group;
	QButtonGroup *replace_standard_devices_group;
};

RKSettingsModuleGraphics::RKSettingsModuleGraphics(QObject *parent) : RKSettingsModule(parent) {
	RK_TRACE(SETTINGS);
}

RKSettingsModuleGraphics::~RKSettingsModuleGraphics() {
	RK_TRACE(SETTINGS);
}

void RKSettingsModuleGraphics::createPages(RKSettings *parent) {
	parent->addSettingsPage(new RKSettingsPageGraphics(parent, this));
}

void RKSettingsModuleGraphics::syncConfig(KConfig *config, RKConfigBase::ConfigSyncAction a) {
	RK_TRACE(SETTINGS);

	KConfigGroup cg = config->group(QStringLiteral("Graphics Device Windows"));
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

// static
QStringList RKSettingsModuleGraphics::makeRRunTimeOptionCommands() {
	RK_TRACE(SETTINGS);
	QStringList list;

	// register RK as interactive
	list.append(QStringLiteral("try (if (!(\"RKGraphicsDevice\" %in% deviceIsInteractive())) deviceIsInteractive(name=\"RKGraphicsDevice\"))\n"));

	QString command = QStringLiteral("options (device=");
	if (default_device == RKDevice) command.append(u"\"RK\""_s);
	else if (default_device == OtherDevice) command.append(RObject::rQuote(default_device_other));
	else {
#if defined Q_OS_WIN
		command.append(u"\"windows\""_s);
#elif defined Q_OS_MACOS
		command.append(u"ifelse (capabilities(\"quartz\"), \"quartz\", \"X11\")"_s);
#else
		command.append(u"\"X11\""_s);
#endif
	}
	list.append(command + u")\n"_s);

	command = u"options (rk.override.platform.devices=\""_s;
	if ((replace_standard_devices == ReplaceDevice) && (default_device != PlatformDevice)) {
		command.append(u"replace"_s);
	} else if (replace_standard_devices == LeaveDevice) {
		command.append(u"nointegration"_s);
	} else {
		command.append(u"embed"_s);
	}
	list.append(command + u"\")\n"_s);

	command = u"options"_s;
	command.append(u"(\"rk.screendevice.width\"="_s + QString::number(graphics_width));
	command.append(u", \"rk.screendevice.height\"="_s + QString::number(graphics_height) + u")\n"_s);
	list.append(command);
	list.append(u"options (\"rk.graphics.hist.max.plotsize\"="_s + QString::number(graphics_hist_max_plotsize) + u")\n"_s);
	list.append(u"rk.toggle.plot.history ("_s + QString(graphics_hist_enable ? u"TRUE"_s : u"FALSE"_s) + u")\n"_s);
	list.append(u"rk.verify.plot.hist.limits ("_s + QString::number(graphics_hist_max_length) + u")\n"_s);

	return (list);
}
