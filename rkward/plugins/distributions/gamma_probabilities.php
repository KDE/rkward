<?
function preprocess () {
}

function calculate () {
	global $q;
	$q = "c (" . preg_replace ("/[, ]+/", ", ", getRK_val ("q")) . ")";
?>
rk.temp = (pgamma (q = <? echo ($q); ?>, shape = <? getRK ("shape"); ?>, rate = <? getRK ("rate"); ?>, <? getRK ("tail"); ?>, <? getRK("logp"); ?>))
<?
}

function printout () {
	global $q;
?>
rk.header ("Gamma probabilities", list ("Vector of quantiles", "<? echo ($q); ?>", "Shape", "<? getRK ("shape"); ?>", "Rate", "<? getRK ("rate"); ?>", "Tail", "<? getRK ("tail"); ?>", "Probabilities p are given as", "<? getRK ("logp"); ?>"))
cat ("<h3>Gamma probabilities:  ", rk.temp, "</h3>")
<?
}

function cleanup () {
?>
rm (rk.temp)
<?
}
?>
