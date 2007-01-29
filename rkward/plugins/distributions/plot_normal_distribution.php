<?
	function preprocess () {
	}

	function calculate () {
	}

	function printout () {

	$fun = getRK_val ("function");
	if ($fun == "dnorm") {
		$label = "density";
		$lower_tag = "";
		$tail_tag = "";
	} else {
		$label = "distribution";
		if (getRK_val("lower") == "1") {
			$lower_tag = ", lower.tail = 1";
			$tail_tag = ", \"Tail\",\"Lower\"";
		} else {
			$lower_tag = ", lower.tail = 0";
			$tail_tag = ", \"Tail\",\"Upper\"";
		}
	}
	if (getRK_val ("log") == "1") $log_label="logarithmic";
	else $log_label="normal";

?>rk.header ("Normal <? echo ($label); ?> function", list ("Number of Observations", "<? getRK ("n"); ?>", "Minimum", "<? getRK ("min"); ?>", "Maximum", "<? getRK ("max"); ?>", "Mean", "<? getRK ("mean"); ?>", "Standard Deviation", "<? getRK ("sd"); ?>", "Non-centrality", "<? getRK ("ncp"); ?>", "Scaling", "<? echo ($log_label); ?>"<? echo ($tail_tag); ?>, "Function", "<? getRK ("function"); ?>"));

rk.graph.on ()
try (plot (function (x) <? getRK ("function"); ?> (x, mean = <? getRK ("mean"); ?>, sd = <? getRK ("sd"); ?>, log = <? getRK ("log"); ?><? echo ($lower_tag); ?>), from=<? getRK ("min"); ?>, to=<? getRK ("max"); ?>, n=<? getRK ("n"); ?>))
rk.graph.off ()
<?
	}

	function cleanup () {
	}
?>
