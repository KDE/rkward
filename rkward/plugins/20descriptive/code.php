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
?>	try (rk.temp.results[[col]]$mean <- mean (rk.temp.vars[col], 0, na.rm=TRUE))
<?
		}
		if ($domedian) {
?>	try (rk.temp.results[[col]]$median <- median (rk.temp.vars[[col]], na.rm=TRUE))
<?
		}
		if ($dorange) {
?>	try (rk.temp.results[[col]]$range <- range (rk.temp.vars[col], na.rm=TRUE))
<?
		}
		if ($dosd) {
?>	try (rk.temp.results[[col]]$sd <- sd (rk.temp.vars[col], na.rm=TRUE))
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
	
		// produce the output
?>
rk.temp.labels <- c (<? echo ("\"" . join ("\",\"", explode ("\n", trim (getRK_val ("x.label")))) . "\""); ?>)
cat ("<h1>Descriptive statistics</h1>")
cat ("<table border=\"1\"><tr><td>Variable</td>")
<? if ($domean) { ?>
cat ("<td>mean</td>")
<? }
	if ($domedian) { ?>
cat ("<td>median</td>")
<? }
	if ($dorange) { ?>
cat ("<td>min</td><td>max</td>")
<? }
	if ($dosd) { ?>
cat ("<td>standard deviation</td>")
<? } ?>
cat ("</tr>")
for (i in 1:dim (rk.temp.vars)[2]) {
	cat (paste ("<tr><td>", rk.temp.labels[i], "</td>"))
<? if ($domean) { ?>
	cat (paste ("<td>", rk.temp.results[[i]]$mean, "</td>"))
<? }
if ($domedian) { ?>
	cat (paste ("<td>", rk.temp.results[[i]]$median, "</td>"))
<? }
if ($dorange) { ?>
	cat (paste ("<td>", rk.temp.results[[i]]$range[1], "</td>", "<td>", rk.temp.results[[i]]$range[2], "</td>"))
<? }
if ($dosd) { ?>
	cat (paste ("<td>", rk.temp.results[[i]]$sd, "</td>"))
<? } ?>
	cat ("</tr>")
}
cat ("</table>")
<?
	}
	
	function cleanup () {
?>rm (rk.temp.vars)
rm (rk.temp.results)
<?
	}
?>
