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
vars <- list (<? echo ($vars); ?>)
results <- data.frame ('Object'=rep (NA, length (vars)))
for (i in 1:length (vars)) {
	results[i, 'Object'] <- rk.get.description (vars[[i]], is.substitute=TRUE)
	var <- eval (vars[[i]], envir=globalenv())	# fetch the real object

	# we wrap each single call in a "try" statement to always continue on errors.
<?
	if (getRK_val ("mean")) { ?>
	results[i, 'mean'] <- try (mean (var, trim = <?echo ($trim) ;?>, na.rm=TRUE))
<?	}
	if (getRK_val ("median")) { ?>
	results[i, 'median'] <- try (median (var, na.rm=TRUE))
<?	}
	if (getRK_val ("range")) { ?>
	try ({
		range <- try (range (var, na.rm=TRUE))
		results[i, 'min'] <- range[1]
		results[i, 'max'] <- range[2]
	})
<?	}
	if (getRK_val ("sd")) { ?>
	results[i, 'standard deviation'] <- try (sd (var, na.rm=TRUE))
<?	}
	if (getRK_val ("sum")) { ?>
	results[i, 'sum'] <- try (sum (var, na.rm=TRUE))
<?	}
	if (getRK_val ("prod")) { ?>
	results[i, 'product'] <- try (prod (var, na.rm=TRUE))
<?	}
	if (getRK_val ("mad")) { ?>
	results[i, 'Median Absolute Deviation'] <- try (mad (var, constant = <? echo ($constMad);
		if ($mad_type == "low") echo (", low=TRUE");
		elseif ($mad_type == "high") echo (", high=TRUE"); ?>, na.rm=TRUE))
<?	}
	if (getRK_val ("length")) { ?>
	results[i, 'length of sample'] <- length (var)
	results[i, 'number of NAs'] <- sum (is.na(var))
<?	} ?>
}
<?
}
	
function printout () {
	global $mad_type;
	global $constMad;
?>
rk.header ("Descriptive statistics", parameters=list (
               "Trim of mean", <?getRK ("trim") ;?><? if (getRK_val ("mad")) { ?>,
               "Median Absolute Deviation",
               paste ("constant:", <?echo ($constMad) ;?>, <?
	if ($mad_type == "low") echo ('"lo-median"');
	elseif ($mad_type == "high") echo ('"hi-median"');
	else echo ('"average"'); ?>)<? } ?>))

rk.results (results)
<?
}

?>
