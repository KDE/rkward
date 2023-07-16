/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
function preprocess() {
	if (getBoolean("beta")) echo ('require("lm.beta")\n');
}

function calculate () {
	var vars = trim (getValue ("x")).replace (/\n/g, " + ");
	var intercept = "";
	if (!getBoolean ("intercept")) intercept = "0 + ";

	var savemodel = getBoolean ("savemodel.active");
	var savefitted = getBoolean ("savefitted.active");
	var saveresiduals = getBoolean ("saveresiduals.active");
	var anova = getBoolean("anova");
	var simple_mode = !(savefitted || saveresiduals || savemodel || anova);

	model = 'lm (' + getValue ("y") + ' ~ ' + intercept + vars;
	if (!simple_mode) model += ', na.action=na.exclude';	// default action of na.omit is a nuisance for fitted values
	model += ')';
	if (getBoolean("beta")) model = 'lm.beta (' + model + ')';

	if (simple_mode) {
		echo ('results <- summary (' + model + ')\n');
	} else {
		echo ('model <- ' + model + '\n');
		if (savemodel) echo ('.GlobalEnv$' + getString ('savemodel') + ' <- model\n');
		if (savefitted) echo ('.GlobalEnv$' + getString ('savefitted') + ' <- fitted (model)\n');
		if (saveresiduals) echo ('.GlobalEnv$' + getString ('saveresiduals') + ' <- residuals (model)\n');
		echo ('results <- summary (model)\n');
		if (anova) echo ('results_anova <- anova (model)\n');
	}
}

function printout (is_preview) {
	if (!is_preview) {
		echo ('rk.header (' + i18n ("Linear Regression") + ')\n');
	}
	echo ('rk.print(results)\n');
	if (getBoolean("anova")) echo ('rk.print(results_anova)\n');
}
