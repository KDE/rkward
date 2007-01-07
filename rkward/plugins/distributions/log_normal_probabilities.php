<?
function preprocess () {
}

function calculate () {
	global $q;
	$q = "c (" . preg_replace ("/[, ]+/", ", ", getRK_val ("q")) . ")";
?>
rk.temp <- (plnorm (q = <? echo ($q); ?>, meanlog = <? getRK ("meanlog"); ?>, sdlog = <? getRK ("sdlog"); ?>, <? getRK ("tail"); ?>, <? getRK ("logp"); ?>))
<?
}

function printout () {
	global $q;
?>
rk.header ("Log Normal probability", list ("Vector of quantiles", "<? echo ($q); ?>", "meanlog", "<? getRK ("meanlog"); ?>", "sdlog", "<? getRK ("sdlog"); ?>", "Tail", "<? getRK ("tail"); ?>", "Probabilities p are given as", "<? getRK ("logp"); ?>"))
rk.results (rk.temp, titles="Log Normal probabilities")
<?
}

function cleanup () {
?>
rm (rk.temp)
<?
}
?>
