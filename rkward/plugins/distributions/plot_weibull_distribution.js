// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	options['args'] = ', shape=' + getString ("shape") + ', scale=' + getString ("scale");
	getContRangeParameters ();

	options['distname'] = i18nc ("Weibull distribution", noquote ("Weibull"));
	if (options['is_density']) {
		options['fun'] = "dweibull";
	} else {
		options['fun'] = "pweibull";
	}
}

function addParamsToHeader (header) {
	header.addFromUI ("shape");
	header.addFromUI ("scale");
	return header;
}
