<?
include ("plot_dist_common.php");

function getParameters () {
	global $options;

	$options['rate'] = getRK_val ("rate");
	getContRangeParameters ();

	if ($options['is_density']) {
		$options['fun'] = "dexp";
	} else {
		$options['fun'] = "pexp";
	}
}

function doHeader () {
	global $options;

	echo ('rk.header ("Exponential ' . $options['label'] . ' function", list ("Number of Observations", "' . $options['n'] . '", "Lower quantile", "' . $options['min'] . '", "Upper quantile", "' . $options['max'] . '", "Rate", "' . $options['rate'] . '"' . $options['log_label'] . $options['tail_label'] . ', "Function", "' . $options['fun'] . '"));' . "\n");
}

function doFunCall () {
	global $options;

	echo ($options['fun'] . '(x, rate=' . $options['rate'] . $options['log_option'] . $options['tail_option'] . ')');
}
?>
