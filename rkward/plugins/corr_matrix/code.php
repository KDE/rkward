<?
	function preprocess () {
	}
	
	function calculate () {
		$vars = "substitute (" . str_replace ("\n", "), substitute (", trim (getRK_val ("x"))) . ")";

?>rk.temp.objects <- list (<? echo ($vars); ?>)
rk.temp.options <- list (method="<? getRK ("method"); ?>", use="<? getRK ("use"); ?>")

rk.temp.frame <- list ()
for (i in 1:length (rk.temp.objects)) rk.temp.frame[[i]] <- eval (rk.temp.objects[[i]])
rk.temp.frame <- data.frame (rk.temp.frame)

rk.temp <- cor (rk.temp.frame, use=rk.temp.options$use, method=rk.temp.options$method)
<?
	}
	
	function printout () {
?>
rk.temp <- as.data.frame (rk.temp)

cat ("<h1>Correlation Matrix</h1>")
cat (paste ("<h2>TODO: describe in verbatim:", rk.temp.options$method, rk.temp.options$use, "</h2>"))

cat ("<table border=\"1\">\n<tr>\n<td>Variable</td>")
for (i in 1:dim (rk.temp)[2]) {
	cat (paste ("<td>", rk.get.short.name (rk.temp.objects[[i]]), "</td>"))
}
cat ("</tr>\n")

for (i in 1:dim (rk.temp)) {
	cat (paste ("<tr><td>", rk.get.description (rk.temp.objects[[i]]), "</td>"))
	for (j in 1:dim (rk.temp)) {
		cat (paste ("<td>", rk.temp[i,j], "</td>"))
	}
	cat ("</tr>\n")
}
cat ("</table>")
<?
	}
	
	function cleanup () {
?>rm (rk.temp)
<?
	}
?>
