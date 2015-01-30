// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	options['args'] = ', df1=' + getString ("df1") + ', df2=' + getString ("df2") + ', ncp=' + getString ("ncp");
	getContRangeParameters ();

	options['distname'] = i18nc ("F distribution", noquote ("F"));
	if (options['is_density']) {
		options['fun'] = "df";
	} else {
		options['fun'] = "pf";
	}
}

function addParamsToHeader (header) {
	header.addFromUI ("df1");
	header.addFromUI ("df2");
	header.addFromUI ("ncp");
	return header;
}
