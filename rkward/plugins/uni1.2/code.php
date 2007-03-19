<?
function preprocess () {
}

function calculate () {
$vars = "substitute (" . str_replace ("\n", "), substitute (", trim (getRK_val ("z"))) . ")";
if (getRK_val ("narm")) $narm = "na.rm=TRUE";
else $name = "na.rm=FALSE";
?>
vars <- list (<? echo ($vars); ?>)
results <- data.frame ('Variable Name'=rep (NA, length (vars)), check.names=FALSE)

for (i in 1:length (vars))  {
	var <- eval (vars[[i]], envir=globalenv());
	results[i, 'Variable Name'] <- rk.get.description(vars[[i]], is.substitute=TRUE)

<?	if (getRK_val ("nombre")) { ?>
	results[i, 'Number of obs'] <- length(var)
<?	}
	if (getRK_val ("nbna")) { ?>
	results[i, 'Number of missing values'] <- length(which(is.na(var)))
<?	}
	if (getRK_val ("mean")) { ?>
	results[i, 'Mean'] <- mean(var,<? echo ($narm); ?>)
<?	}
	if (getRK_val ("vari")) { ?>
	results[i, 'Variance'] <- var(var,<? echo ($narm); ?>)
<?	}
	if (getRK_val ("sd")) { ?>
	results[i, 'Sd'] <- sd(var,<? echo ($narm); ?>)
<?	}
	if (getRK_val ("minimum")) { ?>
	results[i, 'Minimum'] <- min(var,<? echo ($narm); ?>)
<?	}
	if (getRK_val ("maximum")) { ?>
	results[i, 'Maximum'] <- max(var,<? echo ($narm); ?>)
<?	}
	if (($nmin = getRK_val ("nbminimum")) != "0") { ?>
	if (length (var) >= <? echo ($nmin); ?>) {
		results[i, 'Minimum values'] <- paste (sort(var, decreasing=FALSE, na.last=TRUE) [1:<? echo ($nmin); ?>])
	}
<? 	}
	if (($nmax = getRK_val ("nbmaximum")) != "0") { ?>
	if (length (var) >= <? echo ($nmin); ?>) {
		results[i, 'Maximum values'] <- paste (sort(var, decreasing=TRUE, na.last=TRUE) [1:<? echo ($nmin); ?>])
	}
<? 	}
	if (getRK_val ("median")) { ?>
	results[i, 'Median'] <- median(var,<? echo ($narm); ?>)
<?	}
	if (getRK_val ("irq")) { ?>
	results[i, 'Inter Quartile Range'] <- IQR(var,<? echo ($narm); ?>)
<?	}
	if (getRK_val ("quartile")) { ?>
	temp <- quantile (var,<? echo ($narm); ?>)
	results[i, 'Quartiles'] <- paste (names (temp), temp, sep=": ", collapse=" ")
<?	}
	if (($nautre = getRK_val ("autre")) != "0") { ?>
	temp <- quantile (var, probs=seq (0, 1, length.out=<? echo ($nautre); ?>), <? echo ($narm); ?>)
	results[i, 'Quantiles'] <- paste (names (temp), temp, sep=": ", collapse=" ")
<?	} ?>
	
	#robust statistics
<?	if (getRK_val ("trim") == "1") { ?>
	results[i, 'Trimmed Mean'] <- mean (var, trim=<? getRK ("pourcent"); ?>, <? echo ($narm); ?>)
<?	}
	if (getRK_val ("mad") == "1") { ?>
	results[i, 'Median Absolute Deviation'] <- mad (var, constant=<? getRK ("constMad"); ?>, <? echo ($narm); ?>)
<?	}
	if (getRK_val ("huber") == "1") { ?>
	require ("MASS")
	temp <- list (c('Location Estimate','Mad scale estimate'), c(NA,NA))
	try({
		temp <- hubers (var, k = <? getRK ("winsor"); ?>,tol=<? getRK ("tol"); ?><?
	if (getRK_val("customMu")=="1") echo (", mu=".getRK_val("mu"));
	if (getRK_val("customS")=="1") echo (", s=".getRK_val("s"));
	echo (",initmu =".getRK_val("initmu")."(rk.temp.var)") ?>)
	})
	results[i, 'Huber M-Estimator'] <- paste (temp[[1]], temp[[2]], sep=": ", collapse=" ")
<?	} ?>
}

<?	if (getRK_val ("result") == "1") { ?>
# store results
'<? getRK ("nom"); ?>' <- results
<?	} ?>
<?
}

function printout () {
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

rk.results (results)
<?
}
?>
