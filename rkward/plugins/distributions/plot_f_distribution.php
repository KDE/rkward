<?
include ("plot_dist_common.php");

function getParameters () {
	global $options;

	$options['df1'] = getRK_val ("df1");
	$options['df2'] = getRK_val ("df2");
	$options['ncp'] = getRK_val ("ncp");
	getContRangeParameters ();

	if ($options['is_density']) {
		$options['fun'] = "df";
	} else {
		$options['fun'] = "pf";
	}
}

function doHeader () {
	global $options;

	echo ('rk.header ("F ' . $options['label'] . ' function", list ("Number of Observations", "' . $options['n'] . '", "Lower quantile", "' . $options['min'] . '", "Upper quantile", "' . $options['max'] . '", "Numerator degrees of freedom", "' . $options['df1'] . '", "Denominator degrees of freedom", "' . $options['df2'] . '", "Non-centrality", "' . $options['ncp'] . '"' . $options['log_label'] . $options['tail_label'] . ', "Function", "' . $options['fun'] . '"));' . "\n");
}

function doFunCall () {
	global $options;

	echo ($options['fun'] . '(x, df1=' . $options['df1'] . ', df2=' . $options['df2'] . ', ncp=' . $options['ncp'] . $options['log_option'] . $options['tail_option'] . ')');
}
?>
