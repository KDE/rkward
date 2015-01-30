// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	options['args'] = ', shape=' + getString ("shape") + ', rate=' + getString ("rate");
	getContRangeParameters ();

	options['distname'] = i18nc ("Gamma distribution", noquote ("Gamma"));
	if (options['is_density']) {
		options['fun'] = "dgamma";
	} else {
		options['fun'] = "pgamma";
	}
}

function addParamsToHeader (header) {
	header.addFromUI ("shape");
	header.addFromUI ("rate");
	return header;
}
