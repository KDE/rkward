<?
	function preprocess () {
	}
	
	function calculate () {
		$method = getRK_val ("method");
		$use = getRK_val ("use");
		$vars = strtr (trim (getRK_val ("x")), "\n", ",");
		
?>
rk.temp <- cor (data.frame (<? echo ($vars); ?>), NULL, "<? echo $use; ?>", "<? echo $method; ?>")
<?
	}
	
	function printout () {
		$method = getRK_val ("method");
		$use = getRK_val ("use");

		// produce the output
?>
rk.temp.vars <- c (<? echo ("\"" . join ("\",\"", explode ("\n", trim (getRK_val ("x.shortname")))) . "\""); ?>)
rk.temp.labels <- c (<? echo ("\"" . join ("\",\"", explode ("\n", trim (getRK_val ("x.label")))) . "\""); ?>)
rk.temp <- as.data.frame (rk.temp)
cat ("<h1>Correlation Matrix</h1>")
cat ("<h2>TODO: describe in verbatim: <? echo ($method); ?>, <? echo ($use); ?></h2>")
cat ("<table border=\"1\">\n<tr>\n<td>Variable</td>")
for (i in 1:dim (rk.temp)) {
	cat (paste ("<td>", rk.temp.vars[i], "</td>"))
}
cat ("</tr>\n")
for (i in 1:dim (rk.temp)) {
	cat (paste ("<tr><td>", rk.temp.labels[i], "</td>"))
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
