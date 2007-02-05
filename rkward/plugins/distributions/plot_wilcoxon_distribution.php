<?
	function preprocess () {
	}

	function calculate () {
	}

	function printout () {
	doPrintout (true);
}

function cleanup () {
}

function preview () {
	preprocess ();
	calculate ();
	doPrintout (false);
	cleanup ();
}

function doPrintout ($final) {

	$fun = getRK_val ("function");
	$log_option = "";
	if ($fun == "dwilcox") {
		$label = "density";
		$lower_tag = "";
		$tail_tag = "";
		if (getRK_val ("log")) $log_option = ", log=TRUE";
	} else {
		$label = "distribution";
		if (getRK_val("lower") == "1") {
			$lower_tag = ", lower.tail = TRUE";
			$tail_tag = ", \"Tail\",\"Lower\"";
		} else {
			$lower_tag = ", lower.tail = FALSE";
			$tail_tag = ", \"Tail\",\"Upper\"";
		}
		if (getRK_val ("log")) $log_option = ", log.p=TRUE";
	}
	$min = getRK_val ("min");
	$max = getRK_val ("max");
	$nm = getRK_val ("nm");
	$nn = getRK_val ("nn");
	$n = $max - $min + 1;
	if (getRK_val ("log") == "1") $log_label="logarithmic";
	else $log_label="normal";

	if ($final) { ?>
rk.header ("Wilcoxon <? echo ($label); ?> function", list ("Lower quantile", "<? echo ($min); ?>","Upper quantile", "<? echo ($max); ?>", "First sample size", "<? echo ($nm); ?>", "Second sample size", "<? echo ($nn); ?>", "Scaling", "<? echo ($log_label); ?>"<? echo ($tail_tag); ?>, "Function", "<? echo ($fun); ?>"));

rk.graph.on ()
<? }
?>
try (curve (<? echo ($fun); ?> (x, m = <? echo ($nm); ?>, n = <? echo ($nn); ?><? echo ($log_option) ?><? echo ($lower_tag); ?>), from=<? echo ($min); ?>, to=<? echo ($max); ?>, n=<? echo ($n); ?><? getRK ("plotoptions.code.printout"); ?>))

<?	if ($final) { ?>
rk.graph.off ()
<? }
}
?>
