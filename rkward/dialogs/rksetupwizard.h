/***************************************************************************
                          rksetupwizard  -  description
                             -------------------
    begin                : Fri May 25 20200
    copyright            : (C) 2020 by Thomas Friedrichsmeier
    email                : thomas.friedrichsmeier@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef RKSETUPWIZARD_H
#define RKSETUPWIZARD_H

#include <KAssistantDialog>

class QGridLayout;
class QLabel;
class RKSetupWizardItem;
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
private:
	QStringList software_to_install;
	QStringList software_to_install_urls;
	QStringList packages_to_install;
	void createStandardPage();
	QWidget *current_page;
	int current_row;
	QGridLayout* current_layout;
	void appendItem(RKSetupWizardItem* item);
	QList<RKSetupWizardItem*> items;

	void next() override;
	KPageWidgetItem *second_to_last_page_ref;
	QLabel* last_page_label;
};

class QComboBox;
class RKSetupWizardItem {
public:
	enum Status {
		Error,
		Warning,
		Good
	};
	RKSetupWizardItem(const QString &shortlabel, const QString &longlabel=QString(), Status status=Good, const QString &shortstatuslabel=QString()) : status(status), shortlabel(shortlabel), longlabel(longlabel), shortstatuslabel(shortstatuslabel), box(nullptr) {};
	~RKSetupWizardItem() {};
	void addOption(const QString &shortlabel, const QString &longlabel, std::function<void(RKSetupWizard*)> callback) {
		options.append(Option(shortlabel, longlabel, callback));
	}
	void setStatus(Status _status, const QString &_shortstatuslabel) { status = _status; shortstatuslabel = _shortstatuslabel; };
	void setShortLabel(const QString &label) { shortlabel = label; };
	void setLongLabel(const QString &label) { longlabel = label; };
private:
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
