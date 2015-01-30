// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	options['args'] = ', shape1=' + getString ('a') + ', shape2=' + getString ('b') + ', ncp=' + getString ('ncp');
	getContRangeParameters ();

	options['distname'] = i18nc ("Beta distribution", noquote ("Beta"));
	if (options['is_density']) {
		options['fun'] = "dbeta";
	} else {
		options['fun'] = "pbeta";
	}
}

function addParamsToHeader (header) {
	header.addFromUI ("a");
	header.addFromUI ("b");
	header.addFromUI ("ncp");
	return header;
}
