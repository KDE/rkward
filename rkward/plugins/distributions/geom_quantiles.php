<?
function preprocess () {
}

function calculate () {
	global $p;
	$p = "c (" . preg_replace ("/[, ]+/", ", ", getRK_val ("p")) . ")";
?>
result <- (qgeom (p = <? echo ($p); ?>, prob = <? getRK ("prob"); ?>, <? getRK ("tail"); ?>, <? getRK("logp"); ?>))
<?
}

function printout () {
	global $p;
?>
rk.header ("Geometric quantile", list ("Vector of probabilities", "<? echo ($p); ?>", "Probability of success in each trial", "<? getRK ("prob"); ?>", "Tail", "<? getRK ("tail"); ?>", "Probabilities p are given as", "<? getRK ("logp"); ?>"))
rk.results (result, titles="Geometric quantiles")
<?
}
?>
