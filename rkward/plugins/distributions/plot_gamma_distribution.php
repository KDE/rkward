<?
	function preprocess () {
	}

	function calculate () {
	}

	function printout () {

	$fun = getRK_val ("function");
	if ($fun == "dchisq") {
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
	if (getRK_val ("log") == "1") $log_label="logarithmic";
	else $log_label="normal";

?>rk.header ("Plot density <? getRK ("function"); ?>", list ("Number of Observations", "<? getRK ("n"); ?>", "Minimum", "<? getRK ("min"); ?>","Maximum", "<? getRK ("max"); ?>", "Shape", "<? getRK ("shape"); ?>", "Rate", "<? getRK ("rate"); ?>", "Scaling", "<? echo ($log_label); ?>"<? echo ($tail_tag); ?>, "Function", "<? getRK ("function"); ?>"));

rk.graph.on ()
try (plot (<? getRK ("function"); ?> (seq(<? getRK ("min"); ?> ,<? getRK ("max"); ?>, length= <? getRK ("n"); ?>) , shape = <? getRK ("shape"); ?>, rate = <? getRK ("rate"); ?>)))
rk.graph.off ()
<?
	}

	function cleanup () {
	}
?>
