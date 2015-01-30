// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	options['args'] = ', prob=' + getString ("prob");
	getDiscontRangeParameters();

	options['distname'] = i18nc ("Geometric distribution", noquote ("Geometric"));
	if (options['is_density']) {
		options['fun'] = "dgeom";
	} else {
		options['fun'] = "pgeom";
	}
}

function addParamsToHeader (header) {
	header.addFromUI ("prob");
	return header;
}
