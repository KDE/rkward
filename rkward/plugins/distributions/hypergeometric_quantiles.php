<?
function preprocess () {
}

function calculate () {
	global $p;
	$p = "c (" . preg_replace ("/[, ]+/", ", ", getRK_val ("p")) . ")";
?>
rk.temp = (qhyper (p = <? echo ($p); ?>, m = <? getRK ("m"); ?>, n = <? getRK ("n"); ?>, k = <? getRK ("k"); ?>, <? getRK ("tail"); ?>, <? getRK("logp"); ?>))
<?
}

function printout () {
	global $p;
?>
rk.header ("Hypergeometric quantile", list ("Vector of probabilities", "<? echo ($p); ?>", "Number of white balls in the urn", "<? getRK ("m"); ?>", "Number of black balls in the urn", "<? getRK ("n"); ?>", "Number of balls drawn from the urn", "<? getRK ("k"); ?>", "Tail", "<? getRK ("tail"); ?>", "Probabilities p are given as", "<? getRK ("logp"); ?>"))
cat ("<h3>Hypergeometric quantiles:  ", rk.temp, "</h3>")
<?
}

function cleanup () {
?>
rm (rk.temp)
<?
}
?>
