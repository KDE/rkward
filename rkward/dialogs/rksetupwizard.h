/*
rksetupwizard - This file is part of the RKWard project. Created: Fri May 25 2020
SPDX-FileCopyrightText: 2020-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKSETUPWIZARD_H
#define RKSETUPWIZARD_H

#include <functional>
#include <KAssistantDialog>

class QGridLayout;
class QLabel;
class RKSetupWizardItem;
class RKSetupWizardPage;

class RKSetupWizard : public KAssistantDialog {
protected:
	enum InvokationReason {
		ProblemsDetected,
		NewVersionRKWard,
		NewVersionR,
		ManualCheck
	};

	RKSetupWizard(QWidget* parent, InvokationReason reason, const QList<RKSetupWizardItem*> &settings_items);
	~RKSetupWizard();

	static bool has_been_run;
public:
	static void doAutoCheck();
	static void fullInteractiveCheck(InvokationReason reason, const QList<RKSetupWizardItem*> &settings_items = QList<RKSetupWizardItem*>());
	static void manualCheck();

	void markSoftwareForInstallation(const QString &name, const QString &download_url, bool install);
	void markRPackageForInstallation(const QString &name, bool install);
	void next() override;
private:
	QStringList software_to_install;
	QStringList software_to_install_urls;
	QStringList packages_to_install;
	QStringList r_commands_to_run;
friend class RKSetupWizardPage;
	QMap<KPageWidgetItem*, std::function<bool()>> next_callbacks;
	QList<RKSetupWizardItem*> items;
	bool reinstallation_required;
	InvokationReason reason;
	static bool wizard_active;
};

class QComboBox;
class RKSetupWizardItem {
public:
	enum Status {
		Error,
		Warning,
		Good
	};
	explicit RKSetupWizardItem(const QString &shortlabel, const QString &longlabel=QString(), Status status=Good, const QString &shortstatuslabel=QString()) : status(status), shortlabel(shortlabel), longlabel(longlabel), shortstatuslabel(shortstatuslabel), box(nullptr) {};
	~RKSetupWizardItem() {};
	void addOption(const QString &shortlabel, const QString &longlabel, std::function<void(RKSetupWizard*)> callback) {
		options.append(Option(shortlabel, longlabel, callback));
	}
	void setStatus(Status _status, const QString &_shortstatuslabel) { status = _status; shortstatuslabel = _shortstatuslabel; };
	void setShortLabel(const QString &label) { shortlabel = label; };
	void setLongLabel(const QString &label) { longlabel = label; };
private:
friend class RKSetupWizardPage;
friend class RKSetupWizard;
	void createWidget(QGridLayout *layout, int row);
	void apply(RKSetupWizard *wizard);

	struct Option {
		Option(const QString &shortlabel, const QString &longlabel, std::function<void(RKSetupWizard*)> callback) : shortlabel(shortlabel), longlabel(longlabel), callback(callback) {};
		QString shortlabel;
		QString longlabel;
		std::function<void(RKSetupWizard*)> callback;
	};
	QList<Option> options;
	Status status;
	QString shortlabel;
	QString longlabel;
	QString shortstatuslabel;
	QComboBox *box;
};

#endif
