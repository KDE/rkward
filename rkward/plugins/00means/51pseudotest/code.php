<?
	function preprocess () {
	}
	
	function calculate () {
?>rk.temp <- "Nothing of interest"
<?
	}
	
	function printout () {
		// fetch values from R
		$result = callR_val ("print (rk.temp)");
		list ($dummy, $result) = explode ("]", $result, 2);
		$result = trim ($result);
		
		// produce the output
?><h1>Hi!</h1>
<h2>Using PHP you can easily produce fancy output.</h2>
<p>But here's the result in plain text: "<? echo ($result); ?>"</p><?
	}
	
	function cleanup () {
		callR_val ("rm (rk.temp)");
	}
?>
