<?
	function preprocess () {
	}
	
	function calculate () {
?>rk.temp <- t.test (<? getRK ("x"); ?>, <? getRK ("y"); ?>, "<? getRK ("hypothesis"); ?>")
<?
	}
	
	function printout () {
		// fetch values from R
		$t = callR_val ("print (rk.temp\$statistic)");
		$df = callR_val ("print (rk.temp\$parameter)");
		$p = callR_val ("print (rk.temp\$p.value)");
		$est = callR_val ("print (rk.temp\$estimate)");

		// fetch further values from RK
		$x = getRK_val ("x");
		$xlabel = getRK_val ("x.label");
		$y = getRK_val ("y");
		$ylabel = getRK_val ("y.label");
		$hypothesis = getRK_val ("hypothesis");
		if ($hypothesis == "two.sided") {
			$hypothesis = $x . " and " . $y . " differ (two sided)";
		} else if ($hypothesis == "less") {
			$hypothesis = $y . " is greater than " . $x;
		} else {
			$hypothesis = $x . " is greater than " . $y;
		}
		
		// get rid of superflous information / formatting
		list ($dummy, $t) = explode ("\n", $t, 2);
		$t = trim ($t);
		list ($dummy, $df) = explode ("\n", $df, 2);
		$df = trim ($df);
		list ($dummy, $p) = explode (" ", $p, 2);
		$p = trim ($p);
		list ($dummy, $est) = explode ("\n", $est, 2);
		list ($estx, $esty) = explode (" ", trim ($est), 2);
		$estx = trim ($estx);
		$esty = trim ($esty);

		// produce the output
?><h1>T-test (independent samples)</h1>
<h2>Comparing <? echo ($x); ?> (<? echo ($xlabel); ?>) against <? echo ($y); ?> (<? echo ($ylabel); ?>)</h2>
<h3>H1: <? echo ($hypothesis); ?><h3>
<table border="1">
	<tr><td>Variable</td><td>estimated mean</td><td>degrees of freedom</td><td>t</td><td>p</td></tr>
	<tr><td><? echo ($x . "<br>(" . $xlabel . ")"); ?></td><td><? echo ($estx); ?></td><td rowspan="2"><? echo ($df); ?></td><td rowspan="2"><? echo ($t); ?></td><td rowspan="2"><? echo ($p); ?></td></tr>
	<tr><td><? echo ($y . "<br>(" . $ylabel . ")"); ?></td><td><? echo ($esty); ?></td><tr>
</table><?
	}
	
	function cleanup () {
		callR_val ("rm (rk.temp)");
	}
?>
