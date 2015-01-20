var mode;
var values;
var logp;
var lowert;
var dist;

function calculate () {
	mode = getString ("mode");
	if (mode == "q") values = getList ("p.0");
	else values = getList ("q.0");
	if (values.length > 1) values = 'c (' + values.join (', ') + ')';

	logp = getBoolean ("logp.state");
	var logparam = logp ? ', logp=TRUE' : '';
	lowert = getBoolean ("lower.state");
	var tailparam = lowert ? '' : ', lower.tail=FALSE';

	dist = getDistSpecifics ();

	var plabel = logp ? i18n ('Log Probability') : i18n ('Probability');
	var qlabel = i18n ('Quantile');
	var dlabel = i18n ('Density');

	echo ('result <- data.frame (');
	echo (mode == "q" ? plabel : qlabel); 
	echo ('=' + values + ', ');
	if (mode == "d") echo (dlabel + '=d' + dist["funstem"] + ' (' + values + dist["params"] + logparam + ')');                  // NOTE: param lower.tail is not applicable for density function
	else if (mode == "p") echo (plabel + '=p' + dist["funstem"] + ' (' + values + dist["params"] + tailparam + logparam + ')');
	else if (mode == "q") echo (qlabel + '=q' + dist["funstem"] + ' (' + values + dist["params"] + tailparam + logparam + ')');
	echo (')\n');
}

function printout () {
	header = dist["header"];
	header.add (i18nc ("Tail of distribution function: lower / upper", 'Tail'), lowert ? i18n ('Lower tail: P[X ≤ x]') : i18n ('Upper tail: P[X > x]'));
	header.print ();
	echo ('rk.print (result)\n');
}
