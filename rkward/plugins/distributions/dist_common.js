/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
var mode;
var logpd;
var dist;
var invar;
var outvar;

var continuous = 1;
var discrete = 2;

// NOTE: range 
function initDistSpecifics (title, stem, params, range, type) {
	var dist = new Object ();
	var header = new Header (title);
	var par = "";
	for (var i = 0; i < params.length; ++i) {
		header.addFromUI (params[i]);
		par += ', ' + params[i] + '=' + getString (params[i]);
	}
	dist["header"] = header;
	dist["params"] = par;
	dist["funstem"] = stem;
	dist["min"] = range[0];
	dist["max"] = range[1];
	dist["cont"] = (type == continuous);
	return dist;
}

function calculate () {
	mode = getString ("mode");
	dist = getDistSpecifics ();
	invar = 'q';
	if (mode == 'q') invar = 'p'
	outvar = mode;

	var params;
	if (mode == "d") {
		logpd = getBoolean ("logd.state");
		params = logpd ? ', log=TRUE' : '';
		// NOTE: param lower.tail is not applicable for density function
	} else {
		logpd = getBoolean ("logp.state");
		params = logpd ? ', log.p=TRUE' : '';
		if (!getBoolean ("lower.state")) {
			params += ', lower.tail=FALSE';
		}
	}

	var values;
	if (mode == "q") values = getList ("p.0");
	else values = getList ("q.0");
	if (values.length < 1) {
		var max_auto_sequence_length;
		if (mode != "q") max_auto_sequence_length = getValue("n_quantiles");
		else max_auto_sequence_length = getValue("n_probabilities");

		if (invar == 'q') {
			if (!dist["cont"]) {
				var span = Number (dist["max"]) - Number (dist["min"]) - 1;
				if (span <= max_auto_sequence_length) {
					values = String (dist["min"]) + ':' + String (dist["max"]);
				} else {
					values = 'seq.int (' + String (dist["min"]) + ', ' + String (dist["max"]) + ', by=' + String (Math.ceil (span / max_auto_sequence_length)) + ')';
				}
			} else {
				if (dist["min"] === undefined) dist["min"] = 'q' + dist["funstem"] + ' (.01' + dist["params"] + ')';
				if (dist["max"] === undefined) dist["max"] = 'q' + dist["funstem"] + ' (.99' + dist["params"] + ')';
				values = 'seq (' + String (dist["min"]) + ', ' + String (dist["max"]) + ', length.out=' + String (max_auto_sequence_length) + ')';
			}
		} else {    // invar == 'p'
			if (logpd) {
				values = '-' + String (max_auto_sequence_length) + ':0';
			} else {
				values = 'seq (0, 1, length.out=' + String (max_auto_sequence_length) + ')';
			}
		}
	} else {
		if (values.length > 1) values = 'c (' + values.join (', ') + ')';
	}

	echo (invar + ' <- ' + values + '\n');
	echo (outvar + ' <- ' + mode + dist["funstem"] + ' (' + invar + dist["params"] + params + ')\n');
}

function getLabel (quantity) {
	if (quantity == "q") return i18n ('Quantile');
	if (quantity == "d") {
		if (logpd) return i18nc ('logarithm of density', 'log (Density)');
		return i18n ('Density');
	}
	// quantity == "p"
	if (logpd) return i18nc ('logarithm of probability', 'log (Probability)');
	return i18n ('Probability');
}

function printout (is_preview) {
	if (!is_preview) {
		header = dist["header"];
		if (mode != "d") {
			header.add (i18nc ("Tail of distribution function: lower / upper", 'Tail'), getBoolean ("lower.state") ? i18n ('Lower tail: P[X ≤ x]') : i18n ('Upper tail: P[X > x]'));
		}
		header.print ();
	}

	echo ('rk.results (data.frame (' + getLabel (invar) + '=' + invar + ', ' + getLabel (outvar) + '=' + outvar + ', check.names=FALSE))\n');
}
