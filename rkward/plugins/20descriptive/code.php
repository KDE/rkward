<?
	function preprocess () {
	}
	
	function calculate () {
		$vars = "substitute (" . str_replace ("\n", "), substitute (", trim (getRK_val ("x"))) . ")";
?>
rk.temp.options <- list (domean=<? getRK ("mean"); ?>, domedian=<? getRK ("median"); ?>, dorange=<? getRK ("range"); ?>, dosd=<? getRK ("sd"); ?>)
rk.temp.results <- list ()
i=0; for (var in list (<? echo ($vars); ?>)) {
	i = i+1
	rk.temp.results[[i]] <- list ()
	rk.temp.results[[i]]$object <- var
	if (rk.temp.options$domean) try (rk.temp.results[[i]]$mean <- mean (eval (var), 0, na.rm=TRUE))
	if (rk.temp.options$domedian) try (rk.temp.results[[i]]$median <- median (eval (var), na.rm=TRUE))
	if (rk.temp.options$dorange) try (rk.temp.results[[i]]$range <- range (eval (var), na.rm=TRUE))
	if (rk.temp.options$dosd) try (rk.temp.results[[i]]$sd <- sd (eval (var), na.rm=TRUE))
}<?
	}
	
	function printout () {
?>
cat ("<h1>Descriptive statistics</h1>")

cat ("<table border=\"1\"><tr><td>Variable</td>")
if (rk.temp.options$domean) cat ("<td>mean</td>")
if (rk.temp.options$domedian) cat ("<td>median</td>")
if (rk.temp.options$dorange) cat ("<td>min</td><td>max</td>")
if (rk.temp.options$dosd) cat ("<td>standard deviation</td>")
cat ("</tr>")

for (i in 1:length (rk.temp.results)) {
	cat (paste ("<tr><td>", rk.get.description (rk.temp.results[[i]]$object), "</td>"))
	if (rk.temp.options$domean) cat (paste ("<td>", rk.temp.results[[i]]$mean, "</td>"))
	if (rk.temp.options$domedian) cat (paste ("<td>", rk.temp.results[[i]]$median, "</td>"))
	if (rk.temp.options$dorange) cat (paste ("<td>", rk.temp.results[[i]]$range[1], "</td>", "<td>", rk.temp.results[[i]]$range[2], "</td>"))
	if (rk.temp.options$dosd) cat (paste ("<td>", rk.temp.results[[i]]$sd, "</td>"))
	cat ("</tr>")
}
cat ("</table>")
<?
	}
	
	function cleanup () {
?>rm (rk.temp.options)
rm (rk.temp.results)
<?
	}
?>
