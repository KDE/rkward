<?
include ("plot_dist_common.php");

function getParameters () {
	global $options;

	$options['mean'] = getRK_val ("mean");
	$options['sd'] = getRK_val ("sd");
	getContRangeParameters ();

	if ($options['is_density']) {
		$options['fun'] = "dnorm";
	} else {
		$options['fun'] = "pnorm";
	}
}

function doHeader () {
	global $options;

	echo ('rk.header ("Normal ' . $options['label'] . ' function", list ("Number of Observations", "' . $options['n'] . '", "Lower quantile", "' . $options['min'] . '", "Upper quantile", "' . $options['max'] . '", "Mean", "' . $options['mean'] . '", "Standard Deviation", "' . $options['sd'] . '"' . $options['log_label'] . $options['tail_label'] . ', "Function", "' . $options['fun'] . '"));' . "\n");
}

function doFunCall () {
	global $options;

	echo ($options['fun'] . '(x, mean=' . $options['mean'] . ', sd=' . $options['sd'] . $options['log_option'] . $options['tail_option'] . ')'); 
}
?>
