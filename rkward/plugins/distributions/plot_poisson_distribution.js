// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	options['args'] = ', lambda=' + getString ("mean");
	getDiscontRangeParameters();

	options['distname'] = i18nc ("Poisson distribution", noquote ("Poisson"));
	if (options['is_density']) {
		options['fun'] = "dpois";
	} else {
		options['fun'] = "ppois";
	}
}

function addParamsToHeader (header) {
	header.addFromUI ("mean");
	return header;
}
