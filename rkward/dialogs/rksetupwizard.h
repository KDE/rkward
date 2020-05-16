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

class RKSetupWizard : public KAssistantDialog {
public:
	RKSetupWizard(QWidget* parent);
	~RKSetupWizard();

	enum InvokationReason {
		ProblemsDetected,
		NewVersionRKWard,
		NewVersionR,
		ManualCheck
	};

	static void doAutoCheck();
	static void fullInteractiveCheck(InvokationReason reason);
	static bool has_been_run;
};

#endif
