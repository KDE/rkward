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
	if ($fun == "dbinom") {
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
	$size = getRK_val ("size");
	$prob = getRK_val ("prob");
	$max = getRK_val ("max");

	if (getRK_val ("log") == "1") $log_label="logarithmic";
	else $log_label="normal";

	if ($final) { ?>
rk.header ("Binomial <? echo ($label); ?> function", list ("Lower quantile", "<? echo ($min); ?>", "Upper quantile", "<? echo ($max); ?>", "Number of trials", "<? echo ($size); ?>", "Probability of success on each trial", "<? echo ($prob); ?>", "Scaling", "<? echo ($log_label); ?>"<? echo ($tail_tag); ?>, "Function", "<? echo ($fun); ?>"));

rk.graph.on ()
<? }
?>
try (plot (function (x) <? echo ($fun); ?> (x, size = <? echo ($size); ?>, prob=<? echo ($prob); ?><? echo ($log_option); ?><? echo ($lower_tag); ?>), from=<? echo ($min); ?>, to=<? echo ($max); ?>, n=<? echo ($max - $min + 1); ?>, type="p"))

<?	if ($final) { ?>
rk.graph.off ()
<? }
}
?>
