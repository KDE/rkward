<?
include ("plot_dist_common.php");

function getParameters () {
	global $options;

	$options['prob'] = getRK_val ("prob");
	getDiscontRangeParameters();

	if ($options['is_density']) {
		$options['fun'] = "dgeom";
	} else {
		$options['fun'] = "pgeom";
	}
}

function doHeader () {
	global $options;

	echo ('rk.header ("Geometric ' . $options['label'] . ' function", list ("Lower quantile", "' . $options['min'] . '", "Upper quantile", "' . $options['max'] . '", "Probability of success on each trial", "' . $options['prob'] . '"' . $options['log_label'] . $options['tail_label'] . ', "Function", "' . $options['fun'] . '"));' . "\n");
}

function doFunCall () {
	global $options;

	echo ($options['fun'] . '(x, prob=' . $options['prob'] . $options['log_option'] . $options['tail_option'] . ')'); 
}
?>
