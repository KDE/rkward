<?
function preprocess () {
}

function calculate () {
	global $gridoptions;
	$gridoptions = "";
	if (getRK_val ("plotoptions.add_grid")) $gridoptions = "try(" . getRK_val ("plotoptions.grid_options.code.printout") . ")";
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
	global $gridoptions;

	$fun = getRK_val ("function");
	$log_option = "";
	if ($fun == "dbeta") {
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
  $a = getRK_val ("a");
  $b = getRK_val ("b");
  $ncp = getRK_val ("ncp");

	if ($final) { ?>
rk.header ("Beta <? echo ($label); ?> function", list ("Number of Observations", "<? echo ($n); ?>", "Lower quantile", "<? echo ($min); ?>","Upper quantile", "<? echo ($max); ?>", "Shape1", "<? echo ($a); ?>", "Shape2", "<? echo ($b); ?>", "Non-cetrality parameter", "<? echo ($ncp); ?>", "Scaling", "<? echo ($log_label); ?>"<? echo ($tail_tag); ?>, "Function", "<? echo ($fun); ?>"));

rk.graph.on ()
<? }
?>
try (curve (<? echo ($fun); ?> (x, shape1 = <? echo ($a); ?>, shape2 = <? echo ($b); ?>, ncp = <? echo ($ncp); ?><? echo ($log_option) ?><? echo ($lower_tag); ?>), from=<? echo ($min); ?>, to=<? echo ($max); ?>, n=<? echo ($n); ?><? getRK ("plotoptions.code.printout"); ?>))
<? echo ($gridoptions); ?>

<?	if ($final) { ?>
rk.graph.off ()
<? }
}
?>
