/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
// globals
var options;

function printout () {
	doPrintout (true);
}

function preview () {
	if (typeof (preprocess) != "undefined") preprocess ();
	if (typeof (calculate) != "undefined") calculate ();
	doPrintout (false);
}

// get the range parameters for the continuous distributions (it's all the same for these)
function getContRangeParameters () {
	options['n'] = getValue ("n");
	options['min'] = getValue ("min");
	options['max'] = getValue ("max");
	options['cont'] = true;
}

// get the range parameters for the discontinuous distributions (it's all the same for these)
function getDiscontRangeParameters () {
	options['min'] = getValue ("min");
	options['max'] = getValue ("max");
	options['n'] = options['max'] - options['min'] + 1;
	options['cont'] = false;
}

function doPrintout (full) {
	var fun = getString ("function");
	var _log = getBoolean ("log.state");

	var is_density = false;
	var params = "";
	if (fun == "d") {
		is_density = true;
		if (_log) params += ", log=TRUE";
	} else {
		label = "distribution";
		if (getBoolean ("lower.state")) {
			params += ", lower.tail = TRUE";
		} else {
			params += ", lower.tail = FALSE";
		}
		if (_log) params += ", log.p=TRUE";
	}

	options = new Array();
	options['is_density'] = is_density;

	getParameters ();

	if (full) {
	  	var header = new Header (options['is_density'] ? i18nc ("[Some distribution] density function", "%1 density function", options['distname']) : i18nc ("[Some distribution] distribution function", "%1 distribution function", options['distname']));
		if (options['cont']) header.addFromUI ('n');
		header.addFromUI ('min');
		header.addFromUI ('max');
		header = addParamsToHeader (header);
		if (!options['is_density']) {
			header.add (i18nc ("Tail of distribution function: lower / upper", 'Tail'), getBoolean ("lower.state") ? i18n ('Lower tail: P[X ≤ x]') : i18n ('Upper tail: P[X > x]'))
		}
		header.addFromUI ("log");
		header.add (i18n ("Function"), options['fun']);
		header.print ();
		echo ('\n');
		echo ('rk.graph.on ()\n');
	}

	echo ('try ({\n');
	printIndentedUnlessEmpty ("\t", getValue ("plotoptions.code.preprocess"), '', '\n');

	echo ('	curve (');
	echo (options['fun'] + '(x' + options['args'] + params + ')');
	echo (', from=' + options['min'] + ', to=' + options['max'] + ', n=' + options['n'] + getValue ("plotoptions.code.printout") + ')\n');

	printIndentedUnlessEmpty ("\t", getValue ("plotoptions.code.calculate"), '\n', '');
	echo ('})\n');
	if (full) {
		echo ('rk.graph.off ()\n');
	}
}
