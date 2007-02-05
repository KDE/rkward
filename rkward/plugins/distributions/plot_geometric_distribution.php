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
	if ($fun == "dgeom") {
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
	$prob = getRK_val ("prob");
	$max = getRK_val ("max");
	if (getRK_val ("log") == "1") $log_label="logarithmic";
	else $log_label="normal";
	$type = getRK_val ("plotoptions.pointtype.string");
	$type_tag = "";
	if ($type == "")  $type_tag = ", type=\"p\"";

	if ($final) { ?>
rk.header ("Geometric <? echo ($label); ?> function", list ("Lower quantile", "<? echo ($min); ?>", "Upper quantile", "<? echo ($max); ?>", "Probability of success on each trial", "<? echo ($prob); ?>", "Scaling", "<? echo ($log_label); ?>"<? echo ($tail_tag); ?>, "Function", "<? echo ($fun); ?>"));

rk.graph.on ()
<? }
?>
try (curve (<? echo ($fun); ?> (x, prob=<? echo ($prob); ?><? echo ($log_option) ?><? echo ($lower_tag); ?>), from=<? echo ($min); ?>, to=<? echo ($max); ?>, n=<? echo ($max - $min + 1); ?><? getRK ("plotoptions.code.printout"); ?><? echo ($type_tag); ?>))

<?	if ($final) { ?>
rk.graph.off ()
<? }
}
?>
