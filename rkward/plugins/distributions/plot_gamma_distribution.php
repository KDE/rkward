<?
include ("plot_dist_common.php");

function getParameters () {
	global $options;

	$options['shape'] = getRK_val ("shape");
	$options['rate'] = getRK_val ("rate");
	getContRangeParameters ();

	if ($options['is_density']) {
		$options['fun'] = "dgamma";
	} else {
		$options['fun'] = "pgamma";
	}
}

function doHeader () {
	global $options;

	echo ('rk.header ("Gamma ' . $options['label'] . ' function", list ("Number of Observations", "' . $options['n'] . '", "Lower quantile", "' . $options['min'] . '", "Upper quantile", "' . $options['max'] . '", "Shape", "' . $options['shape'] . '", "Rate", "' . $options['rate'] . '"' . $options['log_label'] . $options['tail_label'] . ', "Function", "' . $options['fun'] . '"));' . "\n");
}

function doFunCall () {
	global $options;

	echo ($options['fun'] . '(x, shape=' . $options['shape'] . ', rate=' . $options['rate'] . $options['log_option'] . $options['tail_option'] . ')'); 
}
?>
