<?
include ("plot_dist_common.php");

function getParameters () {
	global $options;

	$options['loc'] = getRK_val ("loc");
	$options['scale'] = getRK_val ("scale");
	getContRangeParameters ();

	if ($options['is_density']) {
		$options['fun'] = "dcauchy";
	} else {
		$options['fun'] = "pcauchy";
	}
}

function doHeader () {
	global $options;

	echo ('rk.header ("Cauchy ' . $options['label'] . ' function", list ("Number of Observations", "' . $options['n'] . '", "Lower quantile", "' . $options['min'] . '", "Upper quantile", "' . $options['max'] . '", "Location", "' . $options['loc'] . '", "Scale", "' . $options['scale'] . '"' . $options['log_label'] . $options['tail_label'] . ', "Function", "' . $options['fun'] . '"));' . "\n");
}

function doFunCall () {
	global $options;

	echo ($options['fun'] . '(x, location=' . $options['loc'] . ', scale=' . $options['scale'] . $options['log_option'] . $options['tail_option'] . ')'); 
}
?>
