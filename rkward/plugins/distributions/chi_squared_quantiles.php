<?
function preprocess () {
}

function calculate () {
	global $p;
	$p = "c (" . preg_replace ("/[, ]+/", ", ", getRK_val ("p")) . ")";
?>
result <- (qchisq (p = <? echo ($p); ?>, df = <? getRK ("df"); ?>, ncp = <? getRK ("ncp"); ?>, <? getRK ("tail"); ?>, <? getRK ("logp"); ?>))
<?
}

function printout () {
	global $p;
?>
rk.header ("Chi-squared quantile", list ("Vector of probabilities", "<? echo ($p); ?>", "Degrees of freedom", "<? getRK ("df"); ?>", "non-centrality parameter", "<? getRK ("ncp"); ?>", "Tail", "<? getRK ("tail"); ?>", "Probabilities p are given as", "<? getRK ("logp"); ?>"));
rk.results (result, titles="Chi-squared quantiles")
<?
}
?>
