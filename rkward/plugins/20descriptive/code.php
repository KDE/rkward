<?
	function preprocess () {
	}
	
	function calculate () {
		$vars = "substitute (" . str_replace ("\n", "), substitute (", trim (getRK_val ("x"))) . ")";
		$trim = getRK_val ("trim"); //the fraction (0 to 0.5) of observations to be trimmed from each end of x before the mean is computed
		$low = getRK_val ("low");
		$high = getRK_val ("high");

?>
rk.temp.options <- list (domean=<? getRK ("mean"); ?>, domedian=<? getRK ("median"); ?>, dorange=<? getRK ("range"); ?>, dosd=<? getRK ("sd"); ?>, dosum=<? getRK ("sum"); ?>, doprod=<? getRK ("prod"); ?>, domad=<? getRK ("mad"); ?>, dolength=<? getRK ("length"); ?>, donacount=<? getRK ("nacount"); ?>)
rk.temp.results <- list ()
i=0; for (var in list (<? echo ($vars); ?>)) {
	i = i+1
	rk.temp.results[[i]] <- list ()
	rk.temp.results[[i]]$object <- rk.get.description (var, is.substitute=TRUE)
	if (rk.temp.options$domean) try (rk.temp.results[[i]]$mean <- mean (eval (var), <?echo ($trim) ;?>, na.rm=TRUE))
	if (rk.temp.options$domedian) try (rk.temp.results[[i]]$median <- median (eval (var), na.rm=TRUE))
	if (rk.temp.options$dorange) try (rk.temp.results[[i]]$range <- range (eval (var), na.rm=TRUE))
	if (rk.temp.options$dosd) try (rk.temp.results[[i]]$sd <- sd (eval (var), na.rm=TRUE))
	if (rk.temp.options$dosum) try (rk.temp.results[[i]]$sum <- sum (eval (var), na.rm=TRUE))
	if (rk.temp.options$doprod) try (rk.temp.results[[i]]$prod <- prod (eval (var), na.rm=TRUE))
	if (rk.temp.options$domad) try (rk.temp.results[[i]]$mad <- mad (eval (var), <?echo ($low) ;?>, <?echo ($high) ;?>, na.rm=TRUE))
	if (rk.temp.options$dolength) try (rk.temp.results[[i]]$length <- length (eval (var)))
	if (rk.temp.options$donacount) try (rk.temp.results[[i]]$nacount <- length (which(is.na(eval (var)))))
}<?
	}
	
	function printout () {
?>
cat ("<h1>Descriptive statistics</h1>")
cat ("<h2>Parmeters</h2>")
cat (paste ("<h3>Trim of mean", <?getRK ("trim") ;?>, "</h3>\n"))
if (rk.temp.options$domad) cat (paste ("<h3>Median Absolute Deviation:", "lo-median is", <?getRK ("low") ;?>, "and  hi-median is", <? getRK ("high") ;?>,"</h3>\n"))
cat ("<table border=\"1\"><tr><td>Variable</td>")
if (rk.temp.options$domean) cat ("<td>mean</td>")
if (rk.temp.options$domedian) cat ("<td>median</td>")
if (rk.temp.options$dorange) cat ("<td>min</td><td>max</td>")
if (rk.temp.options$dosd) cat ("<td>standard deviation</td>")
if (rk.temp.options$dosum) cat ("<td>sum</td>")
if (rk.temp.options$doprod) cat ("<td>product</td>")
if (rk.temp.options$domad) cat ("<td>mad</td>")
if (rk.temp.options$dolength) cat ("<td>length of sample</td>")
if (rk.temp.options$donacount) cat ("<td>number of NAs</td>")
cat ("</tr>")

for (i in 1:length (rk.temp.results)) {
	cat ("<tr><td>", rk.temp.results[[i]]$object, "</td>")
	if (rk.temp.options$domean) cat ("<td>", rk.temp.results[[i]]$mean, "</td>")
	if (rk.temp.options$domedian) cat ("<td>", rk.temp.results[[i]]$median, "</td>")
	if (rk.temp.options$dorange) cat ("<td>", rk.temp.results[[i]]$range[1], "</td>", "<td>", rk.temp.results[[i]]$range[2], "</td>")
	if (rk.temp.options$dosd) cat ("<td>", rk.temp.results[[i]]$sd, "</td>")
	if (rk.temp.options$dosum) cat ("<td>", rk.temp.results[[i]]$sum, "</td>")
	if (rk.temp.options$doprod) cat ("<td>", rk.temp.results[[i]]$prod, "</td>")
	if (rk.temp.options$domad) cat ("<td>", rk.temp.results[[i]]$mad, "</td>")
	if (rk.temp.options$dolength) cat ("<td>", rk.temp.results[[i]]$length, "</td>")
	if (rk.temp.options$donacount) cat ("<td>", rk.temp.results[[i]]$nacount, "</td>")
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
