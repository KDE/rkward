// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	options['loc'] = getValue ("loc");
	options['scale'] = getValue ("scale");
	getContRangeParameters ();

	if (options['is_density']) {
		options['fun'] = "dlogis";
	} else {
		options['fun'] = "plogis";
	}
}

function doHeader () {
	echo ('rk.header ("Logistic ' + options['label'] + ' function", list ("Number of Observations", "' + options['n'] + '", "Lower quantile", "' + options['min'] + '", "Upper quantile", "' + options['max'] + '", "Location", "' + options['loc'] + '", "Scale", "' + options['scale'] + '"' + options['log_label'] + options['tail_label'] + ', "Function", "' + options['fun'] + '"));' + "\n");
}

function doFunCall () {
	echo (options['fun'] + '(x, location=' + options['loc'] + ', scale=' + options['scale'] + options['log_option'] + options['tail_option'] + ')');
}

