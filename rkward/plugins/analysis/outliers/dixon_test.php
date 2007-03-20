<?
function preprocess () { ?>
require(outliers)
<?
}

function calculate () {
	$vars = "substitute (" . str_replace ("\n", "), substitute (", trim (getRK_val ("x"))) . ")";
?>

vars <- list (<? echo ($vars); ?>)
results <- data.frame ('Variable Name'=rep (NA, length (vars)), check.names=FALSE)
for (i in 1:length(vars)) {
	results[i, 'Variable Name'] <- rk.get.description (vars[[i]], is.substitute=TRUE)
	var <- eval (vars[[i]], envir=globalenv ())
	results[i, 'Error'] <- tryCatch ({
		# This is the core of the calculation
		t <- dixon.test (var, type = <? getRK ("type"); ?>, opposite = <? getRK ("opposite"); ?>, two.sided = <? getRK ("two_sided"); ?>)
		results[i, 'Dixon Q-statistic'] <- t$statistic["Q"]
		results[i, 'p-value'] <- t$p.value
		results[i, 'Alternative Hypothesis']<- rk.describe.alternative (t)
<?	if (getRK_val ("mean")) { ?>
		results[i, 'Mean'] <- mean (var)
<?	} ?>
<?	if (getRK_val ("sd")) { ?>
		results[i, 'Standard Deviation'] <-  sd (var)
<?	} ?>
<?	if (getRK_val ("median")) { ?>
		results[i, 'Median'] <- median (var)
<?	} ?>
<?	if (getRK_val ("min")) { ?>
		results[i, 'Minimum'] <- min (var)
<?	} ?>
<?	if (getRK_val ("max")) { ?>
		results[i, 'Maximum'] <- max (var)
<?	} ?>
<?	if (getRK_val ("length")) { ?>
		results[i, 'Length'] <- length (var)
<?	}
	if (getRK_val ("nacount")) { ?>
		results[i, 'NAs'] <- length (which(is.na(var)))
<? 	} ?>
		NA				# no error
	}, error=function (e) e$message)	# catch any errors
}
if (all (is.na (results$'Error'))) results$'Error' <- NULL
<?
}

function printout () {
?>
rk.header ("Dixon test for outlier",
	parameters=list ("Type", "<? getRK ("type"); ?>", "Opposite", "<? getRK ("opposite"); ?>", "two-sided", "<? getRK ("two_sided"); ?>"))
rk.results (results)
<?
}

?>
