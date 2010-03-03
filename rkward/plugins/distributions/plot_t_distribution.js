// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	options['ncp'] = getValue ("ncp");
	options['df'] = getValue ("df");
	getContRangeParameters ();

	if (options['is_density']) {
		options['fun'] = "dt";
	} else {
		options['fun'] = "pt";
	}
}

function doHeader () {
	echo ('rk.header ("Student t ' + options['label'] + ' function", list ("Number of Observations", "' + options['n'] + '", "Minimum", "' + options['min'] + '", "Maximum", "' + options['max'] + '", "Degrees of freedom", "' + options['df'] + '", "Non-centrality", "' + options['ncp'] + '"' + options['log_label'] + options['tail_label'] + ', "Function", "' + options['fun'] + '"));' + "\n");
}

function doFunCall () {
	echo (options['fun'] + '(x, df=' + options['df'] + ', ncp=' + options['ncp'] + options['log_option'] + options['tail_option'] + ')');
}

