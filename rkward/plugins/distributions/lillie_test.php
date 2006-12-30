<?
        function preprocess () {
        }

	function calculate () {
	$vars = "substitute (" . str_replace ("\n", "), substitute (", trim (getRK_val ("x"))) . ")";

?>
	require(nortest)

	rk.temp.options <- list (dolength=<? getRK ("length"); ?>, donacount=<? getRK ("nacount"); ?>)

	rk.temp.results <- list ()
	i=0; for (var in list (<? echo ($vars); ?>)) {
	i = i+1
	rk.temp.results[[i]] <- list ()
	rk.temp.results[[i]]$object <- rk.get.description (var, is.substitute=TRUE)
	rk.temp.results[[i]]$lillie <- lillie.test (eval (var))
	if (rk.temp.options$dolength) try (rk.temp.results[[i]]$length <- length (eval (var)))
	if (rk.temp.options$donacount) try (rk.temp.results[[i]]$nacount <- length (which(is.na(eval (var)))))
}


<?
        }
	function printout () {
?>	cat ("<h1>Lilliefors (Kolmogorov-Smirnov) Normality test</h1>\n")

cat ("<table border=\"1\">")
	cat ("<tbody>")
		cat ("<tr>")
			cat ("<td>Variable Name</td>")
			if (rk.temp.options$dolength) cat ("<td>Length</td>")
			if (rk.temp.options$donacount) cat ("<td>NAs</td>")
			cat ("<td>Statistic</td>")
			cat ("<td>p-value</td>")
			cat ("<td>Test</td>")
			cat ("<td>variable</td>")
		cat ("</tr>")
for (i in 1:length (rk.temp.results)) {
		cat ("<tr><td>", rk.temp.results[[i]]$object, "</td>")
		if (rk.temp.options$dolength) cat ("<td>", rk.temp.results[[i]]$length, "</td>")
		if (rk.temp.options$donacount) cat ("<td>", rk.temp.results[[i]]$nacount, "</td>")
		cat (paste ("<td>", rk.temp.results[[i]]$lillie,"</td>"))
		cat ("</tr>")
}
	cat ("</tbody>")
cat ("</table>")


<?
        }
	function cleanup () {

?>
	rm (rk.temp.results)
	rm (var)
<?
        }
?>