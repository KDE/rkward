var mode;
var logpd;
var dist;
var invar;
var outvar;

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
		var max_auto_sequence_length = 20;

		if (invar == 'q') {
			var maxquant = dist["max_quantile"];
			if (typeof (maxquant) != 'undefined') {
				if (maxquant <= max_auto_sequence_length) {
					values = '0:' + String (maxquant);
				} else {
					values = 'seq.int (0, ' + String (maxquant) + ', by=' + String (Math.ceil (maxquant / max_auto_sequence_length)) + ')';
				}
			} else {
				values = 'seq (0, 1, length.out=' + String (max_auto_sequence_length+1) + ')';
			}
		} else {    // invar == 'p'
			if (logpd) {
				values = '-' + String (max_auto_sequence_length) + ':0';
			} else {
				values = 'seq (0, 1, length.out=' + String (max_auto_sequence_length+1) + ')';
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

function printout () {
	header = dist["header"];
	if (mode != "d") {
		header.add (i18nc ("Tail of distribution function: lower / upper", 'Tail'), getBoolean ("lower.state") ? i18n ('Lower tail: P[X ≤ x]') : i18n ('Upper tail: P[X > x]'));
	}
	header.print ();

	echo ('rk.results (data.frame (' + getLabel (invar) + '=' + invar + ', ' + getLabel (outvar) + '=' + outvar + ', check.names=FALSE))\n');
}
