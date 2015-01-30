// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	options['args'] = ', size=' + getString ('size') + ', prob=' + getString ('prob');
	getDiscontRangeParameters();

	options['distname'] = i18nc ("Binomial distribution", noquote ("Binomial"));
	if (options['is_density']) {
		options['fun'] = "dbinom";
	} else {
		options['fun'] = "pbinom";
	}
}

function addParamsToHeader (header) {
	header.addFromUI ("size");
	header.addFromUI ("prob");
	return header;
}
