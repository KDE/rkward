// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	options['shape'] = getValue ("shape");
	options['rate'] = getValue ("rate");
	getContRangeParameters ();

	if (options['is_density']) {
		options['fun'] = "dgamma";
	} else {
		options['fun'] = "pgamma";
	}
}

function doHeader () {
	echo ('rk.header ("Gamma ' + options['label'] + ' function", list ("Number of Observations", "' + options['n'] + '", "Lower quantile", "' + options['min'] + '", "Upper quantile", "' + options['max'] + '", "Shape", "' + options['shape'] + '", "Rate", "' + options['rate'] + '"' + options['log_label'] + options['tail_label'] + ', "Function", "' + options['fun'] + '"));' + "\n");
}

function doFunCall () {
	echo (options['fun'] + '(x, shape=' + options['shape'] + ', rate=' + options['rate'] + options['log_option'] + options['tail_option'] + ')');
}

