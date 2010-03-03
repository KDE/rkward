// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	options['llim'] = getValue ("llim");
	options['ulim'] = getValue ("ulim");
	getContRangeParameters ();

	if (options['is_density']) {
		options['fun'] = "dunif";
	} else {
		options['fun'] = "punif";
	}
}

function doHeader () {
	echo ('rk.header ("Uniform ' + options['label'] + ' function", list ("Number of Observations", "' + options['n'] + '", "Lower quantile", "' + options['min'] + '", "Upper quantile", "' + options['max'] + '", "Minimum", "' + options['llim'] + '", "Maximum", "' + options['ulim'] + '"' + options['log_label'] + options['tail_label'] + ', "Function", "' + options['fun'] + '"));' + "\n");
}

function doFunCall () {
	echo (options['fun'] + '(x, min=' + options['llim'] + ', max=' + options['ulim'] + options['log_option'] + options['tail_option'] + ')');
}

