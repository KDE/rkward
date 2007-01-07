<?
function preprocess () {
}

function calculate () {
	global $q;
	$q = "c (" . preg_replace ("/[, ]+/", ", ", getRK_val ("q")) . ")";
?>
rk.temp <- (pbeta (q = <? echo ($q); ?>, shape1 = <? getRK ("shape1"); ?>, shape2 = <? getRK ("shape2"); ?>, ncp = <? getRK ("ncp"); ?>, <? getRK ("tail"); ?>, <? getRK("logp"); ?>))
<?
}

function printout () {
	global $q;
?>
rk.header ("Beta probability", list ("Vector of quantiles", "<? echo ($q); ?>", "Shape 1", "<? getRK ("shape1"); ?>", "Shape 2", "<? getRK ("shape2"); ?>", "non-centrality parameter (ncp)", "<? getRK ("ncp"); ?>", "Tail", "<? getRK ("tail"); ?>", "Probabilities p are given as", "<? getRK ("logp"); ?>"));
rk.results (rk.temp, titles="Beta probability")
<?
}

function cleanup () {
?>
rm (rk.temp)
<?
}
?>
