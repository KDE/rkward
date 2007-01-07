<?
function preprocess () {
}

function calculate () {
	global $p;
	$p = "c (" . preg_replace ("/[, ]+/", ", ", getRK_val ("p")) . ")";
?>
rk.temp <- (qpois (p = <? echo ($p); ?>, lambda = <? getRK ("lambda"); ?>, <? getRK ("tail"); ?>, <? getRK("logp"); ?>))
<?
}

function printout () {
	global $p;
?>
rk.header ("Poisson quantile", list ("Vector of probabilities", "<? echo ($p); ?>", "Lambda", "<? getRK ("lambda"); ?>", "Tail", "<? getRK ("tail"); ?>", "Probabilities p are given as", "<? getRK ("logp"); ?>"))
rk.results (rk.temp, titles="Poisson quantiles")
<?
}

function cleanup () {
?>
rm (rk.temp)
<?
}
?>
