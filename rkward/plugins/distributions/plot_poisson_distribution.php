<?
include ("plot_dist_common.php");

function getParameters () {
	global $options;

	$options['mean'] = getRK_val ("mean");
	getDiscontRangeParameters();

	if ($options['is_density']) {
		$options['fun'] = "dpois";
	} else {
		$options['fun'] = "ppois";
	}
}

function doHeader () {
	global $options;

	echo ('rk.header ("Poisson ' . $options['label'] . ' function", list ("Lower quantile", "' . $options['min'] . '", "Upper quantile", "' . $options['max'] . '", "Mean", "' . $options['mean'] . '"' . $options['log_label'] . $options['tail_label'] . ', "Function", "' . $options['fun'] . '"));' . "\n");
}

function doFunCall () {
	global $options;

	echo ($options['fun'] . '(x, lambda=' . $options['mean'] . $options['log_option'] . $options['tail_option'] . ')'); 
}
?>
