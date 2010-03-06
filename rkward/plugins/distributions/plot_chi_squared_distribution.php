<?
include ("plot_dist_common.php");

function getParameters () {
	global $options;

	$options['df'] = getRK_val ("df");
	$options['ncp'] = getRK_val ("ncp");
	getContRangeParameters ();

	if ($options['is_density']) {
		$options['fun'] = "dchisq";
	} else {
		$options['fun'] = "pchisq";
	}
}

function doHeader () {
	global $options;

	echo ('rk.header ("Chisquare ' . $options['label'] . ' function", list ("Number of Observations", "' . $options['n'] . '", "Lower quantile", "' . $options['min'] . '", "Upper quantile", "' . $options['max'] . '", "Degrees of freedom", "' . $options['df'] . '", "Non-centrality parameter", "' . $options['ncp'] . '"' . $options['log_label'] . $options['tail_label'] . ', "Function", "' . $options['fun'] . '"));' . "\n");
}

function doFunCall () {
	global $options;

	echo ($options['fun'] . '(x, df=' . $options['df'] . ', ncp=' . $options['ncp'] . $options['log_option'] . $options['tail_option'] . ')');
}
?>
