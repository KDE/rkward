// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	options['scale'] = getValue ("scale");
	options['shape'] = getValue ("shape");
	getContRangeParameters ();

	if (options['is_density']) {
		options['fun'] = "dweibull";
	} else {
		options['fun'] = "pweibull";
	}
}

function doHeader () {
	echo ('rk.header ("Weibull ' + options['label'] + ' function", list ("Number of Observations", "' + options['n'] + '", "Lower quantile", "' + options['min'] + '", "Upper quantile", "' + options['max'] + '", "Shape", "' + options['shape'] + '", "Scale", "' + options['scale'] + '"' + options['log_label'] + options['tail_label'] + ', "Function", "' + options['fun'] + '"));' + "\n");
}

function doFunCall () {
	echo (options['fun'] + '(x, shape=' + options['shape'] + ', scale=' + options['scale'] + options['log_option'] + options['tail_option'] + ')');
}

