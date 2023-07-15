/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
var lambda = "";
var uplab = "";
var upcol = "";
var uplty = "";

function calculate () {
	if (getValue("custom") == 0) {
		lambda = getValue ("lambda");
	} else {
		lambda = getValue ("clambda");
	}

	echo ('x <- ' + getValue("x") + '\n');
	echo ('lambda <- ' +  lambda + "\n");
	echo ('\n');
	echo ('if (any (is.na (x))) stop (' + i18n ("Missing values cannot be handled in Hodrick-Prescott Filter") + ')\n');
	echo ('\n');
	echo ('i <- diag(length(x))\n');
	echo ('trend <- solve(i + lambda * crossprod(diff(i, lag=1, d=2)), x) '); comment ("The HP Filter itself. Thanks to Grant V. Farnsworth");
	echo ('cycle <- x - trend\n');
	echo ('if (is.ts(x)) {\n');
	echo ('	trend <- ts(trend,start(x),frequency=frequency(x))\n');
	echo ('	cycle <- ts(cycle,start(x),frequency=frequency(x))\n');
	echo ('}\n');
	if (getValue("trend_name.active")) {
		echo ('.GlobalEnv$' + getValue("trend_name") + ' <- trend\n');
	}
	if (getValue("cycle_name.active")) {
		echo ('.GlobalEnv$' + getValue("cycle_name") + ' <- cycle\n');
	}

	if (getValue("series_col.color") != "" & getValue("trend_col.color") != "") {
		upcol = ", col=c(\"" + getValue("series_col.color") + "\", \"" + getValue("trend_col.color") + "\")";
	} else if (getValue("series_col.color") != "") {
		upcol = ", col=c(\"" + getValue("series_col.color") + "\", \"black\")";
	} else if (getValue("trend_col.color") != "") {
		upcol = ", col=c(\"black\", \"" + getValue("trend_col.color") + "\")";
	} else {
		upcol = "";
	}

	if (getValue("series_lty") != "" & getValue("trend_lty") != "") {
		uplty = ", lty=c(" + quote (getValue("series_lty")) + ", " + quote (getValue("trend_lty")) + ")";
	} else if (getValue("series_lty") != "") {
		uplty = ", lty=c(" + quote (getValue("series_lty")) + ", \"solid\")";
	} else if (getValue("trend_lty") != "") {
		uplty = ", lty=c(\"solid\", " + quote (getValue("trend_lty")) + ")";
	} else {
		uplty = "";
	}

	if (getValue("uplab.text") == "") {
		uplab = quote (getValue("x") + ", Trend");
	} else if (getValue("uplabisquote") == 1) {
		uplab = quote (getValue("uplab"));
	} else {
		uplab = getValue("uplab");
	}
}

function printout (is_preview) {
	if (!is_preview) {
		new Header (i18n ("Hodrick-Prescott Filter")).add (i18n ("Lambda"), lambda).print ();
	}

	echo ('rk.graph.on ()\n');
	echo ('try({\n');
	echo ('	par(mfrow=c(');
	if (getValue("plot_cycle") == 1) echo ("2");
	else echo ("1");
	echo (',1),mar=c(2,4,2,2)+0.1)\n');
	echo ('	plot.ts(cbind(x, trend), ylab=' +  uplab +  upcol + ',lwd=c(' + getValue("series_lwd") + ',' + getValue("trend_lwd") + ')' +  uplty + ', plot.type="single")\n');
	var downlab = "";
	if (getValue("plot_cycle") == 1) {
		if (getValue("downlab.text") == "") {
			downlab = "\"Cycle\"";
		} else if (getValue("downlabisquote") == 1) {
			downlab = quote (getValue("downlab"));
		} else {
			downlab = getValue("downlab");
		}

		echo ('	plot.ts(cycle, ylab=' +  downlab);
		if (getValue("cycle_col.color") != "") echo (", col=\"" + getValue("cycle_col.color") + "\"");
		echo (', lwd=' + getValue("cycle_lwd"));
		if (getValue("cycle_lty") != "") echo (", lty=\"" + getValue("cycle_lty") + "\"");
		echo (')\n');
	}

	echo ('})\n');
	echo ('rk.graph.off ()\n');
}

