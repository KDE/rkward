<?
include ("plot_dist_common.php");

function getParameters () {
	global $options;

	$options['size'] = getRK_val ("size");
	$options['prob'] = getRK_val ("prob");
	getDiscontRangeParameters();

	if ($options['is_density']) {
		$options['fun'] = "dbinom";
	} else {
		$options['fun'] = "pbinom";
	}
}

function doHeader () {
	global $options;

	echo ('rk.header ("Binomial ' . $options['label'] . ' function", list ("Lower quantile", "' . $options['min'] . '", "Upper quantile", "' . $options['max'] . '", "Number of trials", "' . $options['size'] . '", "Probability of success on each trial", "' . $options['prob'] . '"' . $options['log_label'] . $options['tail_label'] . ', "Function", "' . $options['fun'] . '"));' . "\n");
}

function doFunCall () {
	global $options;

	echo ($options['fun'] . '(x, size=' . $options['size'] . ', prob=' . $options['prob'] . $options['log_option'] . $options['tail_option'] . ')'); 
}
?>
