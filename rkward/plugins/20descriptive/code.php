<?
	function preprocess () {
	}
	
	function calculate () {
		global $mad_type;
		global $constMad;

		$vars = "substitute (" . str_replace ("\n", "), substitute (", trim (getRK_val ("x"))) . ")";
		$trim = getRK_val ("trim"); //the fraction (0 to 0.5) of observations to be trimmed from each end of x before the mean is computed
		$constMad = getRK_val ("constMad");
		$mad_type = getRK_val ("mad_type");

?>
rk.temp.options <- list (domean=<? getRK ("mean"); ?>, domedian=<? getRK ("median"); ?>, dorange=<? getRK ("range"); ?>, dosd=<? getRK ("sd"); ?>, dosum=<? getRK ("sum"); ?>, doprod=<? getRK ("prod"); ?>, domad=<? getRK ("mad"); ?>, dolength=<? getRK ("length"); ?>, donacount=<? getRK ("nacount"); ?>)
rk.temp.vars <- list (<? echo ($vars); ?>)
rk.temp.results <- data.frame ('Object'=rep (NA, length (rk.temp.vars)))
i=0;
for (rk.temp.var in rk.temp.vars) {
	i = i+1
	rk.temp.results$object[i] <- rk.get.description (rk.temp.var, is.substitute=TRUE)
	if (rk.temp.options$domean) rk.temp.results$mean[i] <- try (mean (eval (rk.temp.var), trim = <?echo ($trim) ;?>, na.rm=TRUE))
	if (rk.temp.options$domedian) rk.temp.results$median[i] <- try (median (eval (rk.temp.var), na.rm=TRUE))
	if (rk.temp.options$dorange) {
		rk.temp.results$min[i] <- NA
		rk.temp.results$max[i] <- NA
		try ({
			rk.temp.range <- try (range (eval (rk.temp.var), na.rm=TRUE))
			rk.temp.results$min[i] <- rk.temp.range[1]
			rk.temp.results$max[i] <- rk.temp.range[2]
		})
	}
	if (rk.temp.options$dosd) rk.temp.results$'standard deviation'[i] <- try (sd (eval (rk.temp.var), na.rm=TRUE))
	if (rk.temp.options$dosum) rk.temp.results$sum[i] <- try (sum (eval (rk.temp.var), na.rm=TRUE))
	if (rk.temp.options$doprod) rk.temp.results$prod[i] <- try (prod (eval (rk.temp.var), na.rm=TRUE))
	if (rk.temp.options$domad) rk.temp.results$'Median Absolute Deviation'[i] <- try (mad (eval (rk.temp.var), constant = <? echo ($constMad);
		if ($mad_type == "low") echo (", low=TRUE");
		elseif ($mad_type == "high") echo (", high=TRUE"); ?>, na.rm=TRUE))
	if (rk.temp.options$dolength) rk.temp.results$'length of sample'[i] <- try (length (eval (rk.temp.var)))
	if (rk.temp.options$donacount) rk.temp.results$'number of NAs'[i] <- try (length (which(is.na(eval (rk.temp.var)))))
}<?
	}
	
	function printout () {
		global $mad_type;
		global $constMad;
?>
rk.header ("Descriptive statistics", parameters=list ("Trim of mean", <?getRK ("trim") ;?>,
					if (rk.temp.options$domad) "Median Absolute Deviation",
					if (rk.temp.options$domad) paste ("constant:", <?echo ($constMad) ;?>, <?
						if ($mad_type == "low") echo ('"lo-median"');
						elseif ($mad_type == '"hi-median"');
						else echo ('"average"'); ?>)))

rk.results (rk.temp.results)
<?
	}
	
	function cleanup () {
?>
rm (list=grep ("^rk.temp", ls (), value=TRUE))
<?
	}
?>
