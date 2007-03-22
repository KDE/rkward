<?
function preprocess () {
}

function calculate () {
	global $q;
	$q = "c (" . preg_replace ("/[, ]+/", ", ", getRK_val ("q")) . ")";
?>
result <- (pf (q = <? echo ($q); ?>, df1 = <? getRK ("df1"); ?>, df2 = <? getRK ("df2"); ?>,  ncp = <? getRK ("ncp"); ?>, <? getRK ("tail"); ?>))
<?
}

function printout () {
	global $q;
?>
rk.header ("F probability", list ("Vector of quantiles", "<? echo ($q); ?>", "Numerator degrees of freedom", "<? getRK ("df1"); ?>", "Denominator degrees of freedom", "<? getRK ("df2"); ?>", "non-centrality parameter", "<? getRK ("ncp"); ?>", "Tail", "<? getRK ("tail"); ?>"));
rk.results (result, titles="F probabilities")
<?
}
?>
