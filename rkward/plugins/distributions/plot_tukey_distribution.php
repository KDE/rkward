<?
include ("plot_dist_common.php");

function getParameters () {
	global $options;

	$options['df'] = getRK_val ("df");
	$options['nmeans'] = getRK_val ("nmeans");
	$options['nranges'] = getRK_val ("nranges");
	getContRangeParameters ();

	if ($options['is_density']) {
		// actually, this can never happen in this case, but we add it here, for consistency with the other plugins
		$options['fun'] = "dtukey";
	} else {
		$options['fun'] = "ptukey";
	}
}

function doHeader () {
	global $options;

	echo ('rk.header ("Tukey ' . $options['label'] . ' function", list ("Number of Observations", "' . $options['n'] . '", "Lower quantile", "' . $options['min'] . '", "Upper quantile", "' . $options['max'] . '", "Sample size for range", "' . $options['nmeans'] . '", "Degreed of freedom for s", "' . $options['df'] . '", "Number of groups", "' . $options['nranges'] . '"' . $options['log_label'] . $options['tail_label'] . ', "Function", "' . $options['fun'] . '"));' . "\n");
}

function doFunCall () {
	global $options;

	echo ($options['fun'] . '(x, nmeans=' . $options['nmeans'] . ', df=' . $options['df'] . ', nranges=' . $options['nranges'] . $options['log_option'] . $options['tail_option'] . ')'); 
}
?>
