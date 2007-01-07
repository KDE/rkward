<?
function preprocess () {
}

function calculate () {
	global $q;
	$q = "c (" . preg_replace ("/[, ]+/", ", ", getRK_val ("q")) . ")";
?>
rk.temp <- (pchisq (q = <? echo ($q); ?>, df = <? getRK ("df"); ?>,  ncp = <? getRK ("ncp"); ?>, <? getRK ("tail"); ?>, <? getRK ("logp"); ?>))
<?
}

function printout () {
	global $q;
?>
rk.header ("Chi-squared probability", list ("Vector of quantiles", "<? echo ($q); ?>", "Degrees of Freedom", "<? getRK ("df"); ?>", "non-centrality parameter", "<? getRK ("ncp"); ?>", "Tail", "<? getRK ("tail"); ?>", "Probabilities p are given as", "<? getRK ("logp"); ?>"));
rk.results (rk.temp, titles="Chi-squared probailities")
<?
}

function cleanup () {
?>
rm (rk.temp)
<?
}
?>
