// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	options['size'] = getValue ("size");
	options['prob'] = getValue ("prob");
	getDiscontRangeParameters();

	if (options['is_density']) {
		options['fun'] = "dbinom";
	} else {
		options['fun'] = "pbinom";
	}
}

function doHeader () {
	echo ('rk.header ("Binomial ' + options['label'] + ' function", list ("Lower quantile", "' + options['min'] + '", "Upper quantile", "' + options['max'] + '", "Number of trials", "' + options['size'] + '", "Probability of success on each trial", "' + options['prob'] + '"' + options['log_label'] + options['tail_label'] + ', "Function", "' + options['fun'] + '"));' + "\n");
}

function doFunCall () {
	echo (options['fun'] + '(x, size=' + options['size'] + ', prob=' + options['prob'] + options['log_option'] + options['tail_option'] + ')');
}

