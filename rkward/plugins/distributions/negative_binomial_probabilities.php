<?
function preprocess () {
}

function calculate () {
	global $q;
	$q = "c (" . preg_replace ("/[, ]+/", ", ", getRK_val ("q")) . ")";

?>
rk.temp <- (pnbinom (q = <? echo ($q); ?>, size = <? getRK ("size"); ?>, prob = <? getRK ("prob"); ?>, <? getRK ("tail"); ?>, <? getRK("logp"); ?>))
<?
}

function printout () {
	global $q;
?>
rk.header ("Negative Binomial probability", list ("Vector of quantiles", "<? echo ($q); ?>", "Size", "<? getRK ("size"); ?>", "Probability of success in each trial", "<? getRK ("prob"); ?>", "Tail", "<? getRK ("tail"); ?>", "Probabilities p are given as", "<? getRK ("logp"); ?>"))
rk.results (rk.temp, titles="Negative Binomial probabilities")
<?
}

function cleanup () {
?>
rm (rk.temp)
<?
}
?>
