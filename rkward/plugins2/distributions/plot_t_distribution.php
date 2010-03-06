<?
include ("plot_dist_common.php");

function getParameters () {
	global $options;

	$options['ncp'] = getRK_val ("ncp");
	$options['df'] = getRK_val ("df");
	getContRangeParameters ();

	if ($options['is_density']) {
		$options['fun'] = "dt";
	} else {
		$options['fun'] = "pt";
	}
}

function doHeader () {
	global $options;

	echo ('rk.header ("Student t ' . $options['label'] . ' function", list ("Number of Observations", "' . $options['n'] . '", "Minimum", "' . $options['min'] . '", "Maximum", "' . $options['max'] . '", "Degrees of freedom", "' . $options['df'] . '", "Non-centrality", "' . $options['ncp'] . '"' . $options['log_label'] . $options['tail_label'] . ', "Function", "' . $options['fun'] . '"));' . "\n");
}

function doFunCall () {
	global $options;

	echo ($options['fun'] . '(x, df=' . $options['df'] . ', ncp=' . $options['ncp'] . $options['log_option'] . $options['tail_option'] . ')'); 
}
?>
