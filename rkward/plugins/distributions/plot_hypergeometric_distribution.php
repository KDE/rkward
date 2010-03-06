<?
include ("plot_dist_common.php");

function getParameters () {
	global $options;

	$options['n_val'] = getRK_val ("n_val");
	$options['m'] = getRK_val ("m");
	$options['k'] = getRK_val ("k");
	getDiscontRangeParameters();

	if ($options['is_density']) {
		$options['fun'] = "dhyper";
	} else {
		$options['fun'] = "phyper";
	}
}

function doHeader () {
	global $options;

	echo ('rk.header ("Hypergeometric ' . $options['label'] . ' function", list ("Lower quantile", "' . $options['min'] . '", "Upper quantile", "' . $options['max'] . '", "Number of white balls", "' . $options['m'] . '", "Number of black balls", "' . $options['n_val'] . '", "Number of balls drawn", "' . $options['k'] . '"' . $options['log_label'] . $options['tail_label'] . ', "Function", "' . $options['fun'] . '"));' . "\n");
}

function doFunCall () {
	global $options;

	echo ($options['fun'] . '(x, m=' . $options['m'] . ', n=' . $options['n_val'] . ', k=' . $options['k'] . $options['log_option'] . $options['tail_option'] . ')'); 
}
?>
