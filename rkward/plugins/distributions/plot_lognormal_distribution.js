// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	options['args'] = ', meanlog=' + getString ("mean") + ', sdlog=' + getString ("sd");
	getContRangeParameters ();

	options['distname'] = i18nc ("Log-Normal distribution", noquote ("Log-Normal"));
	if (options['is_density']) {
		options['fun'] = "dlnorm";
	} else {
		options['fun'] = "plnorm";
	}
}

function addParamsToHeader (header) {
	header.addFromUI ("mean");
	header.addFromUI ("sd");
	return header;
}
