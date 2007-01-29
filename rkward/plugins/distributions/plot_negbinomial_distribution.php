<?
	function preprocess () {
	}

	function calculate () {
	}

	function printout () {
	$fun = getRK_val ("function");
	if ($fun == "dnbinom") {
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

?>rk.header ("Negative Binomial <? echo ($label); ?> function", list ("Lower quantile", "<? getRK ("min"); ?>", "Upper quantile", "<? getRK ("max"); ?>", "Target for number of successful trials", "<? echo ($size); ?>", "<? echo ($paramLabel); ?>", "<? echo ($paramVal); ?>", "Scaling", "<? echo ($log_label); ?>"<? echo ($tail_tag); ?>, "Function", "<? getRK ("function"); ?>"));

rk.graph.on ()
try (plot (function (x) <? getRK ("function"); ?> (x, size = <? getRK ("size"); ?><? echo ($paramTag); ?><? echo ($paramVal); ?>, log = <? getRK ("log"); ?><? echo ($lower_tag); ?>), from=<? echo ($min); ?>, to=<? echo ($max); ?>, n=<? echo ($max - $min + 1); ?>, type="p"))
rk.graph.off ()
<?
	}

	function cleanup () {
	}
?>
