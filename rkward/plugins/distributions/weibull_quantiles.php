<?
function preprocess () {
}

function calculate () {
	global $p;
	$p = "c (" . preg_replace ("/[, ]+/", ", ", getRK_val ("p")) . ")";
?>
rk.temp <- (qweibull (p = <? echo ($p); ?>, shape = <? getRK ("shape"); ?>, scale = <? getRK ("scale"); ?>, <? getRK ("tail"); ?>, <? getRK("logp"); ?>))
<?
}

function printout () {
	global $p;
?>
rk.header ("Weibull quantile", list ("Vector of probabilities", "<? echo ($p); ?>", "Shape", "<? getRK ("shape"); ?>", "Scale", "<? getRK ("scale"); ?>", "Tail", "<? getRK ("tail"); ?>", "Probabilities p are given as", "<? getRK ("logp"); ?>"))
cat ("<h3>Weibull quantiles:  ", rk.temp, "</h3>")
<?
}

function cleanup () {
?>
rm (rk.temp)
<?
}
?>
