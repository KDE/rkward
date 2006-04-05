<?
	function preprocess () {
	}
	
	function calculate () {
		$vars = "substitute (" . str_replace ("\n", "), substitute (", trim (getRK_val ("x"))) . ")";
?>
rk.temp.options <- list (doquantile=<? getRK ("samplequantile"); ?>)
rk.temp.results <- list ()
i=0; for (var in list (<? echo ($vars); ?>)) {
	i = i+1
	rk.temp.results[[i]] <- list ()
	rk.temp.results[[i]]$object <- var
	if (rk.temp.options$doquantile) try (rk.temp.results[[i]]$quantile <- quantile (eval (var), na.rm= <? getRK ("narm"); ?>, type = <? getRK ("samplequantile"); ?>, name = <? getRK ("names"); ?>, probs = seq(<? getRK ("probs1"); ?>, <? getRK ("probs2"); ?>, <? getRK ("probs3"); ?>)))
}<?
	}
	
	function printout () {
?>
cat ("<h1>Descriptive statistics</h1>")

cat ("<table border=\"1\"><tr><td>Variable</td>")
if (rk.temp.options$doquantile) cat ("<td>quantile</td>")
cat ("</tr>")

for (i in 1:length (rk.temp.results)) {
	cat (paste ("<tr><td>", rk.get.description (rk.temp.results[[i]]$object), "</td>"))
	if (rk.temp.options$doquantile) cat (paste ("<td>", rk.temp.results[[i]]$quantile, "</td>"))
	cat ("</tr>")
}
cat ("</table>")
<?
	}
	
	function cleanup () {
?>rm (rk.temp.options)
rm (rk.temp.results)
<?
	}
?>
