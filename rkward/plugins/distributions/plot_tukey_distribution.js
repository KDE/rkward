// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	options['df'] = getValue ("df");
	options['nmeans'] = getValue ("nmeans");
	options['nranges'] = getValue ("nranges");
	getContRangeParameters ();

	if (options['is_density']) {
		// actually, this can never happen in this case, but we add it here, for consistency with the other plugins
		options['fun'] = "dtukey";
	} else {
		options['fun'] = "ptukey";
	}
}

function doHeader () {
	echo ('rk.header ("Tukey ' + options['label'] + ' function", list ("Number of Observations", "' + options['n'] + '", "Lower quantile", "' + options['min'] + '", "Upper quantile", "' + options['max'] + '", "Sample size for range", "' + options['nmeans'] + '", "Degreed of freedom for s", "' + options['df'] + '", "Number of groups", "' + options['nranges'] + '"' + options['log_label'] + options['tail_label'] + ', "Function", "' + options['fun'] + '"));' + "\n");
}

function doFunCall () {
	echo (options['fun'] + '(x, nmeans=' + options['nmeans'] + ', df=' + options['df'] + ', nranges=' + options['nranges'] + options['log_option'] + options['tail_option'] + ')');
}

