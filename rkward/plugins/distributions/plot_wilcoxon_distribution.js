// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	options['args'] = ', m=' + getString ("nm") + ', n=' + getString ("nn");
	getDiscontRangeParameters();

	options['distname'] = i18nc ("Wilcoxon Rank Sum distribution", noquote ("Wilcoxon Rank Sum"));
	if (options['is_density']) {
		options['fun'] = "dwilcox";
	} else {
		options['fun'] = "pwilcox";
	}
}

function addParamsToHeader (header) {
	header.addFromUI ("nm");
	header.addFromUI ("nn");
	return header;
}
