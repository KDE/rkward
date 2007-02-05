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

// 	$fun = getRK_val ("function");
	$fun = "ptukey";
	$log_option = "";
	if ($fun == "dtukey") {
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
	if (getRK_val ("log") == "1") $log_label="logarithmic";
	else $log_label="normal";
	$n = getRK_val ("n");
	$min = getRK_val ("min");
	$max = getRK_val ("max");
	$df = getRK_val ("df");
	$nmeans = getRK_val ("nmeans");
	$nranges = getRK_val ("nranges");

	if ($final) { ?>
rk.header ("Tukey <? echo ($label); ?> function", list ("Number of Observations", "<? echo ($n); ?>", "Lower quantile", "<? echo ($min); ?>","Upper quantile", "<? echo ($max); ?>", "Sample size for range", "<? echo ($nmeans); ?>", "Degreed of freedom for s", "<? echo ($df); ?>", "Number of groups", "<? echo ($nranges); ?>", "Scaling", "<? echo ($log_label); ?>"<? echo ($tail_tag); ?>, "Function", "<? echo ($fun); ?>"));

rk.graph.on ()
<? }
?>
try (curve (<? echo ($fun); ?> (x, nmeans = <? echo ($nmeans); ?>, df = <? echo ($df); ?>, nranges = <? echo ($nranges); ?><? echo ($log_option) ?><? echo ($lower_tag); ?>), from=<? echo ($min); ?>, to=<? echo ($max); ?>, n=<? echo ($n); ?><? getRK ("plotoptions.code.printout"); ?>))

<?	if ($final) { ?>
rk.graph.off ()
<? }
}
?>
