<?
function preprocess () {
}
	
function calculate () {
	global $q;
	$q = "c (" . preg_replace ("/[, ]+/", ", ", getRK_val ("q")) . ")";
?>
rk.temp <- (pwilcox (q = <? echo ($q); ?>, m = <? getRK ("m"); ?>, n = <? getRK ("n"); ?>, <? getRK ("tail"); ?>, <? getRK("logp"); ?>))
<?
	}
	
function printout () {
	global $q;
?>
rk.header ("Wilcoxon Rank Sum probability", list ("Vector of quantiles", "<? echo ($q); ?>", "m (Numbers of observations in the first sample)", "<? getRK ("m"); ?>", "n (Numbers of observations in the second sample)", "<? getRK ("n"); ?>", "Tail", "<? getRK ("tail"); ?>", "Probabilities p are given as", "<? getRK ("logp"); ?>"))
cat ("<h3>Wilcoxon Rank Sum probabilities:  ", rk.temp, "</h3>")
<?
	}
	
	function cleanup () {
?>
rm (rk.temp)
<?
	}
?>
