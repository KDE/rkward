<?
function preprocess () {
}

function calculate () {
	global $q;
	$q = "c (" . preg_replace ("/[, ]+/", ", ", getRK_val ("q")) . ")";

?>
rk.temp <- (pexp (q = <? echo ($q); ?>, rate = <? getRK ("rate"); ?>, <? getRK ("tail"); ?>, <? getRK("logp"); ?>))
<?
}

function printout () {
	global $q;
?>
rk.header ("Exponential probabilities", list ("Vector of quantiles", "<? echo ($q); ?>", "Rate", "<? getRK ("rate"); ?>", "Tail", "<? getRK ("tail"); ?>", "Probabilities p are given as", "<? getRK ("logp"); ?>"))
rk.results (rk.temp, titles="Exponential probabilities")
<?
}

function cleanup () {
?>
rm (rk.temp)
<?
}
?>
