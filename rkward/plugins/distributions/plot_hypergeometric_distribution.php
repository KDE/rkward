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

function doPrintout ($final) {

	$fun = getRK_val ("function");
	$log_option = "";
	if ($fun == "dhyper") {
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
	if (getRK_val ("log") == "1") $log_label="logarithmic";
	else $log_label="normal";
	$n = getRK_val ("n");
	$m = getRK_val ("m");
	$k = getRK_val ("k");

	if ($final) { ?>
rk.header ("Hypergeometric <? echo ($label); ?> function", list ("Lower quantile", "<? echo ($min); ?>", "Upper quantile", "<? echo ($max); ?>", "Number of white balls", "<? echo ($m); ?>",  "Number of black balls", "<? echo ($n); ?>", "Number of balls drawn", "<? echo ($k); ?>", "Scaling", "<? echo ($log_label); ?>"<? echo ($tail_tag); ?>, "Function", "<? echo ($fun); ?>"));

rk.graph.on ()
<? }
?>
try (curve (<? echo ($fun); ?> (x, m = <? echo ($m); ?>, n = <? echo ($n); ?>, k = <? echo ($k); ?><? echo ($log_option) ?><? echo ($lower_tag); ?>), from=<? echo ($min); ?>, to=<? echo ($max); ?>, n=<? echo ($max - $min + 1); ?><? getRK ("plotoptions.code.printout"); ?><? echo ($type_tag); ?>))

<?	if ($final) { ?>
rk.graph.off ()
<? }
}
?>
