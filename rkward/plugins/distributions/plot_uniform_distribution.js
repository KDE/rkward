// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	options['args'] = ', min=' + getString ("llim") + ', max=' + getString ("ulim");
	getContRangeParameters ();

	options['distname'] = i18nc ("Uniform distribution", noquote ("Uniform"));
	if (options['is_density']) {
		options['fun'] = "dunif";
	} else {
		options['fun'] = "punif";
	}
}

function addParamsToHeader (header) {
	header.addFromUI ("llim");
	header.addFromUI ("ulim");
	return header;
}
