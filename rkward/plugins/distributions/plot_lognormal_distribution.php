<?
include ("plot_dist_common.php");

function getParameters () {
	global $options;

	$options['sd'] = getRK_val ("sd");
	$options['mean'] = getRK_val ("mean");
	getContRangeParameters ();

	if ($options['is_density']) {
		$options['fun'] = "dlnorm";
	} else {
		$options['fun'] = "plnorm";
	}
}

function doHeader () {
	global $options;

	echo ('rk.header ("Lognormal ' . $options['label'] . ' function", list ("Number of Observations", "' . $options['n'] . '", "Lower quantile", "' . $options['min'] . '", "Upper quantile", "' . $options['max'] . '", "Mean", "' . $options['mean'] . '", "Standard deviation", "' . $options['sd'] . '"' . $options['log_label'] . $options['tail_label'] . ', "Function", "' . $options['fun'] . '"));' . "\n");
}

function doFunCall () {
	global $options;

	echo ($options['fun'] . '(x, meanlog=' . $options['mean'] . ', sdlog=' . $options['sd'] . $options['log_option'] . $options['tail_option'] . ')');
}
?>
