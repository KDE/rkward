// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	options['args'] = ', mean=' + getString ("mean") + ', sd=' + getString ("sd");
	getContRangeParameters ();

	options['distname'] = i18nc ("Normal distribution", noquote ("Normal"));
	if (options['is_density']) {
		options['fun'] = "dnorm";
	} else {
		options['fun'] = "pnorm";
	}
}

function addParamsToHeader (header) {
	header.addFromUI ("mean");
	header.addFromUI ("sd");
	return header;
}
