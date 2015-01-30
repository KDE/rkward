// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	options['args'] = ', rate=' + getString ("rate");
	getContRangeParameters ();

	options['distname'] = i18nc ("Exponential distribution", noquote ("Exponential"));
	if (options['is_density']) {
		options['fun'] = "dexp";
	} else {
		options['fun'] = "pexp";
	}
}

function addParamsToHeader (header) {
	header.addFromUI ("rate");
	return header;
}
