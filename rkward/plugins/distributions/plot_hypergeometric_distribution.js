// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	options['args'] = ', m=' + getString ("m") + ', n=' + getString ("n_val") + ', k=' + getString ("k");
	getDiscontRangeParameters();

	options['distname'] = i18nc ("Hypergeometric distribution", noquote ("Hypergeometric"));
	if (options['is_density']) {
		options['fun'] = "dhyper";
	} else {
		options['fun'] = "phyper";
	}
}

function addParamsToHeader (header) {
	header.addFromUI ("m");
	header.addFromUI ("n_val");
	header.addFromUI ("k");
	return header;
}
