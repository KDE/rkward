<?
	function preprocess () {
	}
	
	function calculate () {
	getRK ("model");
?>rk.temp <- "Nothing of interest"
<?
	}
	
	function printout () {
?><h1>Hi!</h1>
<h2>Using PHP you can easily produce fancy output.</h2>
<p>But here's the result in plain text: "<? echo (callR ("cat (rk.temp)")); ?>"</p><?
	}
	
	function cleanup () {
		callR_val ("rm (rk.temp)");
	}
?>
