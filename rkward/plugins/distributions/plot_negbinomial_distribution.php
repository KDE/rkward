<?
include ("plot_dist_common.php");

function getParameters () {
	global $options;

 	if (getRK_val ("param") == "pprob") {
		$size = getRK_val ("size_trial");
		$size_label = "Target for number of successful trials";
		$paramTag = ", prob=";
		$paramVal = getRK_val ("prob");
		$paramLabel = "Probability of success in each trial";
	} else {
		$size = getRK_val ("size_disp");
		$size_label = "Dispersion (size)";
		$paramTag = ", mu=";
		$paramVal = getRK_val ("mu");
		$paramLabel = "Alternative parameter, mu";
	}

	$options['size'] = $size;
	$options['size_label'] = $size_label;
	$options['param_tag'] = $paramTag;
	$options['param_val'] = $paramVal;
	$options['param_label'] = $paramLabel;
	$options['prob'] = getRK_val ("prob");
	getDiscontRangeParameters();

	if ($options['is_density']) {
		$options['fun'] = "dnbinom";
	} else {
		$options['fun'] = "pnbinom";
	}
}

function doHeader () {
	global $options;

	echo ('rk.header ("Negative Binomial ' . $options['label'] . ' function", list ("Lower quantile", "' . $options['min'] . '", "Upper quantile", "' . $options['max'] . '", "' . $options['size_label'] . '", "' . $options['size'] . '", "' . $options['param_label'] . '", "' . $options['param_val'] . '"' . $options['log_label'] . $options['tail_label'] . ', "Function", "' . $options['fun'] . '"));' . "\n");
}

function doFunCall () {
	global $options;

	echo ($options['fun'] . '(x, size=' . $options['size'] . $options['param_tag'] . $options['param_val'] . $options['log_option'] . $options['tail_option'] . ')'); 
}
?>
