<?
	function preprocess () {
	}
	
	function calculate () {
	getRK ("model");
?>rk.temp <- "Nothing of interest"
<?
	}
	
	function printout () {
	// produce the output
?>cat ("<h1>Hi!</h1>\n")
cat ("<h2>Using R-scripting you can produce fancy output.</h2>\n")
cat (paste ("<p>But here's the result in plain text: ", rk.temp, "</p>\n"))
cat ("<p>Check the R-interface watch to see how it was produced</p>")
<?
	}
	
	function cleanup () {
?>rm (rk.temp)
<?
	}
?>
