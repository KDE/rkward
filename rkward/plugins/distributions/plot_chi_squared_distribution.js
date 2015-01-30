// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	options['args'] = ', df=' + getString ("df") + ', ncp=' + getString ("ncp");
	getContRangeParameters ();

	options['distname'] = i18nc ("Chi-square distribution", noquote ("Chi-square"));
	if (options['is_density']) {
		options['fun'] = "dchisq";
	} else {
		options['fun'] = "pchisq";
	}
}

function addParamsToHeader (header) {
	header.addFromUI ("df");
	header.addFromUI ("ncp");
	return header;
}
