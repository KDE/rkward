<?
        function preprocess () {
        }

	function calculate () {
	$vars = "substitute (" . str_replace ("\n", "), substitute (", trim (getRK_val ("x"))) . ")";

?>
	
	require(moments)

	rk.temp.options <- list (doskewness=<? getRK ("skewness"); ?>, dokurtosis=<? getRK ("kurtosis"); ?>, dolength=<? getRK ("length"); ?>, donacount=<? getRK ("nacount"); ?>)

	rk.temp.results <- list ()
	i=0; for (var in list (<? echo ($vars); ?>)) {
	i = i+1
	rk.temp.results[[i]] <- list ()
	rk.temp.results[[i]]$object <- rk.get.description (var, is.substitute=TRUE)
	if (rk.temp.options$doskewness) try (rk.temp.results[[i]]$skewness <- skewness (eval (var), <? getRK ("narm_skewness"); ?>))
	if (rk.temp.options$dokurtosis) try (rk.temp.results[[i]]$kurtosis <- kurtosis (eval (var), <? getRK ("narm_kurtosis"); ?>))
	if (rk.temp.options$dolength) try (rk.temp.results[[i]]$length <- length (eval (var)))
	if (rk.temp.options$donacount) try (rk.temp.results[[i]]$nacount <- length (which(is.na(eval (var)))))
}


<?
        }
	function printout () {
?>	
	 rk.header("Skewness and Kurtosis")

cat ("<table border=\"1\">")
	cat ("<tbody>")
		cat ("<tr>")
			cat ("<td>Variable Name</td>")
			if (rk.temp.options$dolength) cat ("<td>Length</td>")
			if (rk.temp.options$donacount) cat ("<td>NAs</td>")
			if (rk.temp.options$doskewness) cat ("<td>Skewness</td>")
			if (rk.temp.options$dokurtosis) cat ("<td>Kurtosis</td>")
		cat ("</tr>")
for (i in 1:length (rk.temp.results)) {
		cat ("<tr><td>", rk.temp.results[[i]]$object, "</td>")
		if (rk.temp.options$dolength) cat ("<td>", rk.temp.results[[i]]$length, "</td>")
		if (rk.temp.options$donacount) cat ("<td>", rk.temp.results[[i]]$nacount, "</td>")
		if (rk.temp.options$doskewness) cat (paste ("<td>", rk.temp.results[[i]]$skewness,"</td>"))
		if (rk.temp.options$dokurtosis) cat (paste ("<td>", rk.temp.results[[i]]$kurtosis,"</td>"))
		cat ("</tr>")
}
	cat ("</tbody>")
cat ("</table>")


<?
        }
	function cleanup () {

?>
	rm (rk.temp.results)
	rm (rk.temp.options)
	rm (var)
<?
        }
?>