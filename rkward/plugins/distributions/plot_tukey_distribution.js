// globals
var options;

include ('plot_dist_common.js');

function getParameters () {
	options['args'] = ', nmeans=' + getString ("nmeans") + ', df=' + getString ("df");
	getContRangeParameters ();

	options['distname'] = i18nc ("Tukey distribution", noquote ("Studentized Range (Tukey)"));
	if (options['is_density']) {
		// actually, this can never happen in this case, but we add it here, for consistency with the other plugins
		options['fun'] = "dtukey";
	} else {
		options['fun'] = "ptukey";
	}
}

function addParamsToHeader (header) {
	header.addFromUI ("nmeans");
	header.addFromUI ("df");
	return header;
}
