<?
	function preprocess () {
	}
	
	function calculate () {
		$domean = getRK_val ("mean");
		$domedian = getRK_val ("median");
		$dorange = getRK_val ("range");
		$dosd = getRK_val ("sd");
		$vars = strtr (trim (getRK_val ("x")), "\n", ",");
		
		
?>
rk.temp.vars <- data.frame (<? echo ($vars); ?>)
rk.temp.results <- list ()
for (col in 1:length (rk.temp.vars)) {
	rk.temp.results[[col]] <- list ()
<?

		if ($domean) {
?>	try (rk.temp.results[[col]]$mean <- mean (rk.temp.vars[col], 0, TRUE))
<?
		}
		if ($domedian) {
?>	try (rk.temp.results[[col]]$median <- median (rk.temp.vars[col], TRUE))
<?
		}
		if ($dorange) {
?>	try (rk.temp.results[[col]]$range <- range (rk.temp.vars[col], na.rm=TRUE))
<?
		}
		if ($dosd) {
?>	try (rk.temp.results[[col]]$sd <- sd (rk.temp.vars[col], TRUE))
<?
		}	
?>
}<?
	}
	
	function printout () {
		$domean = getRK_val ("mean");
		$domedian = getRK_val ("median");
		$dorange = getRK_val ("range");
		$dosd = getRK_val ("sd");
		$vars = array ();
		$vars = explode ("\n", trim (getRK_val ("x")));
		$means = array ();
		$medians = array ();
		$mins = array ();
		$maxs = array ();
		$sds = array ();
		
		// fetch values from R and format
		for ($i = 0; $i < count ($vars); ++$i) {
			if ($domean) {
				list ($dummy, $mean) = explode (" ", callR_val ("print (rk.temp.results[[" . ($i + 1) . "]]\$mean)"), 2);
				array_push ($means, trim ($mean));
			}
			if ($domedian) {
				list ($dummy, $median) = explode (" ", callR_val ("print (rk.temp.results[[" . ($i + 1) . "]]\$median)"), 2);
				array_push ($medians, trim ($median));
			}
			if ($dorange) {
				list ($dummy, $range) = explode (" ", callR_val ("print (rk.temp.results[[" . ($i + 1) . "]]\$range)"), 2);
				list ($min, $max) = explode (" ", trim ($range), 2);
				array_push ($mins, $min);
				array_push ($maxs, $max);
			}
			if ($dosd) {
				list ($dummy, $sd) = explode (" ", callR_val ("print (rk.temp.results[[" . ($i + 1) . "]]\$sd)"), 2);
				array_push ($sds, trim ($sd));
			}
		}

		// fetch further values from RK
//		$x = getRK_val ("x");
//		$xlabel = getRK_val ("x.label");
		
		
		// produce the output
?><h1>Descriptive statistics</h1>
<?/*<?<h2>Stats for <? echo ($x); ?> (<? echo ($xlabel); ?>)</h2>*/?>
<table border="1">
	<tr>
	<td>Variable</td>
	<? if ($domean) { ?><td>mean</td><? }
	if ($domedian) { ?><td>median</td><? }
	if ($dorange) { ?><td>min</td><td>max</td><? }
	if ($dosd) { ?><td>standard deviation</td><? } ?>
	</tr>
<? for ($i = 0; $i < count ($vars); ++$i) { ?>
		<tr>
		<td><? echo ($vars[$i] . " (" . $xlabel . ")"); ?></td>
		<? if ($domean) { ?><td><? echo $means[$i]; ?></td><? }
		if ($domedian) { ?><td><? echo $medians[$i]; ?></td><? }
		if ($dorange) { ?><td><? echo $mins[$i]; ?></td><td><? echo $maxs[$i]; ?></td><? }
		if ($dosd) { ?><td><? echo $sds[$i]; ?></td><? } ?>
		</tr>
<?}?>
</table><?
	}
	
	function cleanup () {
		callR_val ("rm (rk.temp.vars)");
		callR_val ("rm (rk.temp.results)");
	}
?>
