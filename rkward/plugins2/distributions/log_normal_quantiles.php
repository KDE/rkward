<?
function preprocess () {
}

function calculate () {
	global $p;
	$p = "c (" . preg_replace ("/[, ]+/", ", ", getRK_val ("p")) . ")";
?>
result <- (qlnorm (p = <? echo ($p); ?>, meanlog = <? getRK ("meanlog"); ?>, sdlog = <? getRK ("sdlog"); ?>, <? getRK ("tail"); ?>, <? getRK ("logp"); ?>))
<?
}

function printout () {
	global $p;
?>
rk.header ("Log Normal quantile", list ("Vector of probabilities", "<? echo ($p); ?>", "meanlog", "<? getRK ("meanlog"); ?>", "sdlog", "<? getRK ("sdlog"); ?>", "Tail", "<? getRK ("tail"); ?>", "Probabilities p are given as", "<? getRK ("logp"); ?>"))
rk.results (result, titles="Log Normal quantiles")
<?
}
?>
