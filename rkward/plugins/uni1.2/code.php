<?
	function preprocess () {
	}
	
	function calculate () {
$vars = "substitute (" . str_replace ("\n", "), substitute (", trim (getRK_val ("z"))) . ")";
if (getRK_val ("narm")) $narm = "na.rm=TRUE";
else $name = "na.rm=FALSE";
?>
rk.temp.vars <- list (<? echo ($vars); ?>)
rk.temp.results <- data.frame ('Variable Name'=rep (NA, length (rk.temp.vars)), check.names=FALSE)

rk.temp.i <- 0
for (rk.temp.var in rk.temp.vars)  {
	rk.temp.i <- rk.temp.i + 1
	rk.temp.results[['Variable Name']][rk.temp.i] <- rk.get.description(rk.temp.var, is.substitute=TRUE)
	rk.temp.var <- eval(rk.temp.var)

<?	if (getRK_val ("nombre")) { ?>
	rk.temp.results[['Number of obs']][rk.temp.i] <- length(rk.temp.var)
<?	}
	if (getRK_val ("nbna")) { ?>
	rk.temp.results[['Number of missing values']][rk.temp.i] <- length(which(is.na(rk.temp.var)))
<?	}
	if (getRK_val ("moyenne")) { ?>
	rk.temp.results[['Mean']][rk.temp.i] <- mean(rk.temp.var,<? echo ($narm); ?>)
<?	}
	if (getRK_val ("vari")) { ?>
	rk.temp.results[['Variance']][rk.temp.i] <- var(rk.temp.var,<? echo ($narm); ?>)
<?	}
	if (getRK_val ("ecartt")) { ?>
	rk.temp.results[['Sd']][rk.temp.i] <- sd(rk.temp.var,<? echo ($narm); ?>)
<?	}
	if (getRK_val ("minimum")) { ?>
	rk.temp.results[['Minimum']][rk.temp.i] <- min(rk.temp.var,<? echo ($narm); ?>)
<?	}
	if (getRK_val ("maximum")) { ?>
	rk.temp.results[['Maximum']][rk.temp.i] <- max(rk.temp.var,<? echo ($narm); ?>)
<?	}
	if (($nmin = getRK_val ("nbminimum")) != "0") { ?>
	if (length (rk.temp.var) >= <? echo ($nmin); ?>) {
		rk.temp.results[['Minimum values']][rk.temp.i] <- paste (sort(rk.temp.var, decreasing=FALSE, na.last=TRUE) [1:<? echo ($nmin); ?>])
	}
<? 	}
	if (($nmax = getRK_val ("nbmaximum")) != "0") { ?>
	if (length (rk.temp.var) >= <? echo ($nmin); ?>) {
		rk.temp.results[['Maximum values']][rk.temp.i] <- paste (sort(rk.temp.var, decreasing=TRUE, na.last=TRUE) [1:<? echo ($nmin); ?>])
	}
<? 	}
	if (getRK_val ("mediane")) { ?>
	rk.temp.results[['Median']][rk.temp.i] <- median(rk.temp.var,<? echo ($narm); ?>)
<?	}
	if (getRK_val ("irq")) { ?>
	rk.temp.results[['Inter Quartile Range']][rk.temp.i] <- IQR(rk.temp.var,<? echo ($narm); ?>)
<?	}
	if (getRK_val ("quartile")) { ?>
	rk.temp.temp <- quantile (rk.temp.var,<? echo ($narm); ?>)
	rk.temp.results[['Quartiles']][rk.temp.i] <- paste (names (rk.temp.temp), rk.temp.temp, sep=": ", collapse=" ")
<?	}
	if (($nautre = getRK_val ("autre")) != "0") { ?>
	rk.temp.temp <- quantile (rk.temp.var, probs=seq (0, 1, length.out=<? echo ($nautre); ?>), <? echo ($narm); ?>)
	rk.temp.results[['Quantiles']][rk.temp.i] <- paste (names (rk.temp.temp), rk.temp.temp, sep=": ", collapse=" ")
<?	} ?>
	
	#robust statistics
<?	if (getRK_val ("trim") == "1") { ?>
	rk.temp.results[['Trimmed Mean']][rk.temp.i] <- mean (rk.temp.var, trim="<? getRK ("pourcent"); ?>", <? echo ($narm); ?>)
<?	}
	if (getRK_val ("mad") == "1") { ?>
	rk.temp.results[['Median Absolute Deviation']][rk.temp.i] <- mad (rk.temp.var, constant = "<? getRK ("constMad"); ?>", <? echo ($narm); ?>)
<?	}
	if (getRK_val ("huber") == "1") { ?>
	require ("MASS")
	rk.temp.temp <- list (c('Location Estimate','Mad scale estimate'), c(NA,NA))
	try({
		rk.temp.temp[[2]] <- hubers (rk.temp.var, k = <? getRK ("winsor"); ?>,tol=<? getRK ("tol"); ?><?
	if (getRK_val("customMu")=="1") echo (", mu=".getRK_val("mu"));
	if (getRK_val("customS")=="1") echo (", s=".getRK_val("s"));
	echo (",initmu =".getRK_val("initmu")."(rk.temp.var)") ?>)
	})
	rk.temp.results[['Huber M-Estimator']][rk.temp.i] <- paste (rk.temp.temp[[1]], rk.temp.temp[[2]], sep=": ", collapse=" ")
<?	} ?>
}

<?	if (getRK_val ("result") == "1") { ?>
# store results
'<? getRK ("nom"); ?>' <- rk.temp.results
<?	} ?>
<?
	}
	
	function printout () {
	// produce the output
?>
rk.header ("Univariate statistics", parameters=list (
"Remove Missing values", <? if (getRK_val ("narm")) echo ("TRUE"); else echo ("FALSE"); ?>
<?	if (getRK_val("trim")=="1") { ?>
, "Trimmed value for trimmed mean", "<? getRK ("pourcent"); ?>"
<?	}
	if (getRK_val("mad")=="1") { ?>
, "Constant for the MAD estimation", "<? getRK ("constMad"); ?>"
<?	}
	if (getRK_val("huber")=="1") { ?>
, "Winsorized values for Huber estimator", "<? getRK ("winsor"); ?>"
, "Tolerance in Huber estimator", "<? getRK ("tol"); ?>"
<?		if (getRK_val ("customMu")=="1") { ?>
, "Mu for Huber estimator", "<? getRK ("mu"); ?>"
<?		}
		if (getRK_val ("customS")=="1") { ?>
, "S for Huber estimator", "<? getRK ("s"); ?>"
<?		} ?>
, "Initial value", "<? getRK ("initmu"); ?>"
<?	} ?>
))

rk.results (rk.temp.results)
<?
	}
	
	function cleanup () {
?>
rm (rk.temp.results)
rm (rk.temp.i)
try (rm (rk.temp.temp))
<?
	}
?>
