<?
function preprocess () {
}

function calculate () {
}

function printout () {
	doPrintout (true);
}

function preview () {
	preprocess ();
	calculate ();
	doPrintout (false);
}

// get the range parameters for the continuous distributions (it's all the same for these)
function getContRangeParameters () {
	global $options;

	$options['n'] = getRK_val ("n");
	$options['min'] = getRK_val ("min");
	$options['max'] = getRK_val ("max");
}

// get the range parameters for the discontinuous distributions (it's all the same for these)
function getDiscontRangeParameters () {
	global $options;

	$options['min'] = getRK_val ("min");
	$options['max'] = getRK_val ("max");
	$options['n'] = $options['max'] - $options['min'] + 1;
}

function doPrintout ($final) {
	global $options;

	$fun = getRK_val ("function");
	$log = (getRK_val ("log") == 1);
	$log_option = "";
	if ($fun == "d") {
		$is_density = true;
		$label = "density";
		$tail_option = "";
		$tail_label = "";
		if ($log) $log_option = ", log=TRUE";
	} else {
		$is_density = false;
		$label = "distribution";
		if (getRK_val("lower") == "1") {
			$tail_option = ", lower.tail = TRUE";
			$tail_label = ", \"Tail\",\"Lower\"";
		} else {
			$tail_option = ", lower.tail = FALSE";
			$tail_label = ", \"Tail\",\"Upper\"";
		}
		if ($log) $log_option = ", log.p=TRUE";
	}
	if ($log) $log_label = ', "Scale", "logarithmic"';
	else $log_label = ', "Scale", "normal"';

	$options = array ();
	$options['is_density'] = $is_density;
	$options['label'] = $label;
	$options['tail_option'] = $tail_option;
	$options['tail_label'] = $tail_label;
	$options['log_option'] = $log_option;
	$options['log_label'] = $log_label;

	getParameters ();

	if ($final) {
		doHeader (); ?>

rk.graph.on ()
<?	}
?>
try ({
	curve (<? doFunCall (); ?>, from=<? echo ($options['min']); ?>, to=<? echo ($options['max']); ?>, n=<? echo ($options['n']); ?><? getRK ("plotoptions.code.printout"); ?>)
<?	
	$plot_adds = getRK_val ("plotoptions.code.calculate");
	if (!empty ($plot_adds)) { ?>

<?
		printIndented ("\t", $plot_adds);
	} ?>
})
<?	if ($final) { ?>
rk.graph.off ()
<? }
}
?>
