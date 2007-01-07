<?
function preprocess () {
}

function calculate () {
	global $p;
	$p = "c (" . preg_replace ("/[, ]+/", ", ", getRK_val ("p")) . ")";
?>
rk.temp <- (qexp (p = <? echo ($p); ?>, rate = <? getRK ("rate"); ?>, <? getRK ("tail"); ?>, <? getRK("logp"); ?>))
<?
}

function printout () {
	global $p;
?>
rk.header ("Exponential quantiles", list ("Vector of probabilities", "<? echo ($p); ?>", "Rate", "<? getRK ("rate"); ?>", "Tail", "<? getRK ("tail"); ?>", "Probabilities p are given as", "<? getRK ("logp"); ?>"))
rk.results (rk.temp, titles="Exponential quantiles")
<?
}

function cleanup () {
?>
rm (rk.temp)
<?
}
?>
