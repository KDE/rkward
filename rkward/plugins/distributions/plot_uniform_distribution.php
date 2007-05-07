<?
include ("plot_dist_common.php");

function getParameters () {
	global $options;

	$options['llim'] = getRK_val ("llim");
	$options['ulim'] = getRK_val ("ulim");
	getContRangeParameters ();

	if ($options['is_density']) {
		$options['fun'] = "dunif";
	} else {
		$options['fun'] = "punif";
	}
}

function doHeader () {
	global $options;

	echo ('rk.header ("Uniform ' . $options['label'] . ' function", list ("Number of Observations", "' . $options['n'] . '", "Lower quantile", "' . $options['min'] . '", "Upper quantile", "' . $options['max'] . '", "Minimum", "' . $options['llim'] . '", "Maximum", "' . $options['ulim'] . '"' . $options['log_label'] . $options['tail_label'] . ', "Function", "' . $options['fun'] . '"));' . "\n");
}

function doFunCall () {
	global $options;

	echo ($options['fun'] . '(x, min=' . $options['llim'] . ', max=' . $options['ulim'] . $options['log_option'] . $options['tail_option'] . ')'); 
}
?>
