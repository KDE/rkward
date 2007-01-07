<?
function preprocess () {
}
	
function calculate () {
	global $p;
	$p = "c (" . preg_replace ("/[, ]+/", ", ", getRK_val ("p")) . ")";
?>
rk.temp <- (qwilcox (p = <? echo ($p); ?>, m = <? getRK ("m"); ?>, n = <? getRK ("n"); ?>, <? getRK ("tail"); ?>, <? getRK("logp"); ?>))
<?
	}
	
function printout () {
	global $p;
?>
rk.header ("Wilcoxon Rank Sum quantile", list ("Vector of probabilities", "<? echo ($p); ?>", "m (Numbers of observations in the first sample)", "<? getRK ("m"); ?>", "n (Numbers of observations in the second sample)", "<? getRK ("n"); ?>", "Tail", "<? getRK ("tail"); ?>", "Probabilities p are given as", "<? getRK ("logp"); ?>"))
rk.results (rk.temp, titles="Wilcoxon Rank Sum quantiles")
<?
	}
	
	function cleanup () {
?>
rm (rk.temp)
<?
	}
?>
