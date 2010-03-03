// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	options['prob'] = getValue ("prob");
	getDiscontRangeParameters();

	if (options['is_density']) {
		options['fun'] = "dgeom";
	} else {
		options['fun'] = "pgeom";
	}
}

function doHeader () {
	echo ('rk.header ("Geometric ' + options['label'] + ' function", list ("Lower quantile", "' + options['min'] + '", "Upper quantile", "' + options['max'] + '", "Probability of success on each trial", "' + options['prob'] + '"' + options['log_label'] + options['tail_label'] + ', "Function", "' + options['fun'] + '"));' + "\n");
}

function doFunCall () {
	echo (options['fun'] + '(x, prob=' + options['prob'] + options['log_option'] + options['tail_option'] + ')');
}

