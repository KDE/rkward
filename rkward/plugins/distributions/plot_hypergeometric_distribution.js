// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	options['n_val'] = getValue ("n_val");
	options['m'] = getValue ("m");
	options['k'] = getValue ("k");
	getDiscontRangeParameters();

	if (options['is_density']) {
		options['fun'] = "dhyper";
	} else {
		options['fun'] = "phyper";
	}
}

function doHeader () {
	echo ('rk.header ("Hypergeometric ' + options['label'] + ' function", list ("Lower quantile", "' + options['min'] + '", "Upper quantile", "' + options['max'] + '", "Number of white balls", "' + options['m'] + '", "Number of black balls", "' + options['n_val'] + '", "Number of balls drawn", "' + options['k'] + '"' + options['log_label'] + options['tail_label'] + ', "Function", "' + options['fun'] + '"));' + "\n");
}

function doFunCall () {
	echo (options['fun'] + '(x, m=' + options['m'] + ', n=' + options['n_val'] + ', k=' + options['k'] + options['log_option'] + options['tail_option'] + ')');
}

