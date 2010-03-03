// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	options['sd'] = getValue ("sd");
	options['mean'] = getValue ("mean");
	getContRangeParameters ();

	if (options['is_density']) {
		options['fun'] = "dlnorm";
	} else {
		options['fun'] = "plnorm";
	}
}

function doHeader () {
	echo ('rk.header ("Lognormal ' + options['label'] + ' function", list ("Number of Observations", "' + options['n'] + '", "Lower quantile", "' + options['min'] + '", "Upper quantile", "' + options['max'] + '", "Mean", "' + options['mean'] + '", "Standard deviation", "' + options['sd'] + '"' + options['log_label'] + options['tail_label'] + ', "Function", "' + options['fun'] + '"));' + "\n");
}

function doFunCall () {
	echo (options['fun'] + '(x, meanlog=' + options['mean'] + ', sdlog=' + options['sd'] + options['log_option'] + options['tail_option'] + ')');
}

