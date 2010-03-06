<?
function preprocess () {
}

function calculate () {
	global $p;
	$p = "c (" . preg_replace ("/[, ]+/", ", ", getRK_val ("p")) . ")";

?>
result <- (qbeta (p = <? echo ($p); ?>, shape1 = <? getRK ("shape1"); ?>, shape2 = <? getRK ("shape2"); ?>, ncp = <? getRK ("ncp"); ?>, <? getRK ("tail"); ?>, <? getRK("logp"); ?>))
<?
}

function printout () {
	global $p;
?>
rk.header ("Beta quantiles", list ("Vector of probabilities", "<? echo ($p); ?>", "Shape 1", "<? getRK ("shape1"); ?>", "Shape 2", "<? getRK ("shape2"); ?>", "non-centrality parameter (ncp)", "<? getRK ("ncp"); ?>", "Tail", "<? getRK ("tail"); ?>", "Probabilities p are given as", "<? getRK ("logp"); ?>"));
rk.results (result, titles="Beta quantiles")
<?
}
?>
