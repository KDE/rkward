<?
	function preprocess () {
	}

	function calculate () {
	}

	function printout () {
	$fun = getRK_val ("function");
	if ($fun == "dpois") {
		$label = "mass";
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
	$min = getRK_val ("min");
	$max = getRK_val ("max");
	if (getRK_val ("log") == "1") $log_label="logarithmic";
	else $log_label="normal";

?>rk.header ("Poisson <? echo ($label); ?> function", list ("Lower quantile", "<? getRK ("min"); ?>", "Upper quantile", "<? getRK ("max"); ?>", "Mean", "<? getRK ("mean"); ?>", "Scaling", "<? echo ($log_label); ?>"<? echo ($tail_tag); ?>, "Function", "<? getRK ("function"); ?>"));

rk.graph.on ()
try (plot (function (x) <? getRK ("function"); ?> (x, lambda = <? getRK ("mean"); ?>, log = <? getRK ("log"); ?><? echo ($lower_tag); ?>), from=<? getRK ("min"); ?>, to=<? getRK ("max"); ?>, n=<? echo ($max - $min + 1); ?>, type="p"))
rk.graph.off ()
<?
	}

	function cleanup () {
	}
?>
