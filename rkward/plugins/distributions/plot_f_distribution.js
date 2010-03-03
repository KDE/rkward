// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	options['df1'] = getValue ("df1");
	options['df2'] = getValue ("df2");
	options['ncp'] = getValue ("ncp");
	getContRangeParameters ();

	if (options['is_density']) {
		options['fun'] = "df";
	} else {
		options['fun'] = "pf";
	}
}

function doHeader () {
	echo ('rk.header ("F ' + options['label'] + ' function", list ("Number of Observations", "' + options['n'] + '", "Lower quantile", "' + options['min'] + '", "Upper quantile", "' + options['max'] + '", "Numerator degrees of freedom", "' + options['df1'] + '", "Denominator degrees of freedom", "' + options['df2'] + '", "Non-centrality", "' + options['ncp'] + '"' + options['log_label'] + options['tail_label'] + ', "Function", "' + options['fun'] + '"));' + "\n");
}

function doFunCall () {
	echo (options['fun'] + '(x, df1=' + options['df1'] + ', df2=' + options['df2'] + ', ncp=' + options['ncp'] + options['log_option'] + options['tail_option'] + ')');
}

