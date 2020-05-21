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
class RKSetupWizardItem;
class RKSetupWizard : public KAssistantDialog {
protected:
	RKSetupWizard(QWidget* parent);
	~RKSetupWizard();

	enum InvokationReason {
		ProblemsDetected,
		NewVersionRKWard,
		NewVersionR,
		ManualCheck
	};
	static bool has_been_run;
public:
	static void doAutoCheck();
	static void fullInteractiveCheck(InvokationReason reason);

	void markExternalPackageForInstallation(const QString &name, bool install);
	void markRPackageForInstallation(const QString &name, bool install);
private:
	QStringList software_to_install;
	QStringList packages_to_install;
	void createStandardPage();
	QWidget *current_page;
	int current_row;
	QGridLayout* current_layout;
	void appendItem(RKSetupWizardItem* item);
	QList<RKSetupWizardItem*> items;
};

#endif
