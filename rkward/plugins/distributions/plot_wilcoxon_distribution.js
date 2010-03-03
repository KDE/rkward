// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	options['nm'] = getValue ("nm");
	options['nn'] = getValue ("nn");
	getDiscontRangeParameters();

	if (options['is_density']) {
		options['fun'] = "dwilcox";
	} else {
		options['fun'] = "pwilcox";
	}
}

function doHeader () {
	echo ('rk.header ("Binomial ' + options['label'] + ' function", list ("Lower quantile", "' + options['min'] + '", "Upper quantile", "' + options['max'] + '", "First sample size", "' + options['nm'] + '", "Second sample size", "' + options['nn'] + '"' + options['log_label'] + options['tail_label'] + ', "Function", "' + options['fun'] + '"));' + "\n");
}

function doFunCall () {
	echo (options['fun'] + '(x, m=' + options['nm'] + ', n=' + options['nn'] + options['log_option'] + options['tail_option'] + ')');
}

