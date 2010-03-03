// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	var size = "";
	var size_label = "";
	var paramTag = "";
	var paramVal = "";
	var paramLabel = "";
	if (getValue ("param") == "pprob") {
		size = getValue ("size_trial");
		size_label = "Target for number of successful trials";
		paramTag = ", prob=";
		paramVal = getValue ("prob");
		paramLabel = "Probability of success in each trial";
	} else {
		size = getValue ("size_disp");
		size_label = "Dispersion (size)";
		paramTag = ", mu=";
		paramVal = getValue ("mu");
		paramLabel = "Alternative parameter, mu";
	}

	options['size'] = size;
	options['size_label'] = size_label;
	options['param_tag'] = paramTag;
	options['param_val'] = paramVal;
	options['param_label'] = paramLabel;
	options['prob'] = getValue ("prob");
	getDiscontRangeParameters();

	if (options['is_density']) {
		options['fun'] = "dnbinom";
	} else {
		options['fun'] = "pnbinom";
	}
}

function doHeader () {
	echo ('rk.header ("Negative Binomial ' + options['label'] + ' function", list ("Lower quantile", "' + options['min'] + '", "Upper quantile", "' + options['max'] + '", "' + options['size_label'] + '", "' + options['size'] + '", "' + options['param_label'] + '", "' + options['param_val'] + '"' + options['log_label'] + options['tail_label'] + ', "Function", "' + options['fun'] + '"));' + "\n");
}

function doFunCall () {
	echo (options['fun'] + '(x, size=' + options['size'] + options['param_tag'] + options['param_val'] + options['log_option'] + options['tail_option'] + ')');
}

