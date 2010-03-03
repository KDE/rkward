// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	options['rate'] = getValue ("rate");
	getContRangeParameters ();

	if (options['is_density']) {
		options['fun'] = "dexp";
	} else {
		options['fun'] = "pexp";
	}
}

function doHeader () {
	echo ('rk.header ("Exponential ' + options['label'] + ' function", list ("Number of Observations", "' + options['n'] + '", "Lower quantile", "' + options['min'] + '", "Upper quantile", "' + options['max'] + '", "Rate", "' + options['rate'] + '"' + options['log_label'] + options['tail_label'] + ', "Function", "' + options['fun'] + '"));' + "\n");
}

function doFunCall () {
	echo (options['fun'] + '(x, rate=' + options['rate'] + options['log_option'] + options['tail_option'] + ')');
}

