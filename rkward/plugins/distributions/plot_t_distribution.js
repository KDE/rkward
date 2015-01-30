// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	options['args'] = ', df=' + getString ("df") + ', ncp=' + getString ("ncp");
	getContRangeParameters ();

	options['distname'] = i18nc ("t distribution", noquote ("Student t"));
	if (options['is_density']) {
		options['fun'] = "dt";
	} else {
		options['fun'] = "pt";
	}
}

function addParamsToHeader (header) {
	header.addFromUI ("df");
	header.addFromUI ("ncp");
	return header;
}
