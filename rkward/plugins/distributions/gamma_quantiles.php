<?
function preprocess () {
}

function calculate () {
	global $p;
	$p = "c (" . preg_replace ("/[, ]+/", ", ", getRK_val ("p")) . ")";
?>
rk.temp <- (qgamma (p = <? echo ($p); ?>, shape = <? getRK ("shape"); ?>, rate = <? getRK ("rate"); ?>, <? getRK ("tail"); ?>, <? getRK("logp"); ?>))

<?
}

function printout () {
	global $p;
?>
rk.header ("Gamma quantile", list ("Vector of probabilities", "<? echo ($p); ?>", "Shape", "<? getRK ("shape"); ?>", "Rate", "<? getRK ("rate"); ?>", "Tail", "<? getRK ("tail"); ?>", "Probabilities p are given as", "<? getRK ("logp"); ?>"))
rk.results (rk.temp, titles="Gamma quantiles")
<?
}

function cleanup () {
?>
rm (rk.temp)
<?
}
?>
