<?
	function preprocess () {
	}
	
	function calculate () {
		$domean = getRK_val ("mean");
		$domedian = getRK_val ("median");
		$dorange = getRK_val ("range");
		$dosd = getRK_val ("sd");
		$x = getRK_val ("x");
	
		?>rk.temp <- list ()
<?
		
		if ($domean) {
		?>rk.temp$mean <- mean (<? echo $x; ?>, 0, TRUE)
<?
		}
		if ($domedian) {
		?>rk.temp$median <- median (<? echo $x; ?>, TRUE)
<?
		}
		if ($dorange) {
		?>rk.temp$range <- range (<? echo $x; ?>, na.rm=TRUE)
<?
		}
		if ($dosd) {
		?>rk.temp$sd <- sd (<? echo $x; ?>, TRUE)
<?
		}	
	}
	
	function printout () {
		$domean = getRK_val ("mean");
		$domedian = getRK_val ("median");
		$dorange = getRK_val ("range");
		$dosd = getRK_val ("sd");
		
		// fetch values from R and format
		if ($domean) {
			list ($dummy, $mean) = explode (" ", callR_val ("print (rk.temp\$mean)"), 2);
			$mean = trim ($mean);
		}
		if ($domedian) {
			list ($dummy, $median) = explode (" ", callR_val ("print (rk.temp\$median)"), 2);
			$median = trim ($median);
		}
		if ($dorange) {
			list ($dummy, $range) = explode (" ", callR_val ("print (rk.temp\$range)"), 2);
			list ($min, $max) = explode (" ", trim ($range), 2);
		}
		if ($dosd) {
			list ($dummy, $sd) = explode (" ", callR_val ("print (rk.temp\$sd)"), 2);
			$sd = trim ($sd);
		}

		// fetch further values from RK
		$x = getRK_val ("x");
		$xlabel = getRK_val ("x.label");
		
		
		// produce the output
?><h1>Descriptive statistics</h1>
<h2>Stats for <? echo ($x); ?> (<? echo ($xlabel); ?>)</h2>
<table border="1">
	<tr>
	<td>Variable</td>
	<? if ($domean) { ?><td>mean</td><? }
	if ($domedian) { ?><td>median</td><? }
	if ($dorange) { ?><td>min</td><td>max</td><? }
	if ($dosd) { ?><td>standard deviation</td><? } ?>
	</tr>
	<tr>
	<td><? echo ($x . " (" . $xlabel . ")"); ?></td>
	<? if ($domean) { ?><td><? echo $mean; ?></td><? }
	if ($domedian) { ?><td><? echo $median; ?></td><? }
	if ($dorange) { ?><td><? echo $min; ?></td><td><? echo $max; ?></td><? }
	if ($dosd) { ?><td><? echo $sd; ?></td><? } ?>
	</tr>
</table><?
	}
	
	function cleanup () {
		callR_val ("rm (rk.temp)");
	}
?>
