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
	if ($fun == "dnbinom") {
		$label = "mass";
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
	$size = getRK_val ("size");
	$paramTag = "";
	$paramVal = "";
 	if (getRK_val ("param") == "pprob") {
		$paramTag = ", prob=";
		$paramVal = getRK_val ("prob");
		$paramLabel = "Probability of success in each trial";
	} else {
		$paramTag = ", mu=";
		$paramVal = getRK_val ("mu");
		$paramLabel = "Alternative parameter, mu";
	}
	if (getRK_val ("log") == "1") $log_label="logarithmic";
	else $log_label="normal";

	if ($final) { ?>
rk.header ("Negative Binomial <? echo ($label); ?> function", list ("Lower quantile", "<? echo ($min); ?>", "Upper quantile", "<? echo ($max); ?>", "Target for number of successful trials", "<? echo ($size); ?>", "<? echo ($paramLabel); ?>", "<? echo ($paramVal); ?>", "Scaling", "<? echo ($log_label); ?>"<? echo ($tail_tag); ?>, "Function", "<? echo ($fun); ?>"));

rk.graph.on ()
<? }
?>
try (plot (function (x) <? echo ($fun); ?> (x, size = <? echo ($size); ?><? echo ($paramTag); ?><? echo ($paramVal); ?><? echo ($log_option) ?><? echo ($lower_tag); ?>), from=<? echo ($min); ?>, to=<? echo ($max); ?>, n=<? echo ($max - $min + 1); ?>, type="p"))

<?	if ($final) { ?>
rk.graph.off ()
<? }
}
?>
