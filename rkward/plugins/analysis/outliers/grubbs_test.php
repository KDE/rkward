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
<?	if (getRK_val ("length")) { ?>
	var <- eval (vars[[i]], envir=globalenv ())

	results[i, 'Length'] <- length (var)
	results[i, 'NAs'] <- sum (is.na(var))

	var <- na.omit (var) 	# omit NAs for all further calculations
<? 	} else { ?>
	var <- na.omit (eval (vars[[i]], envir=globalenv ()))
<?	} ?>

	results[i, 'Error'] <- tryCatch ({
		# This is the core of the calculation
		t <- grubbs.test (var, type = <? getRK ("type"); ?>, opposite = <? getRK ("opposite"); ?>, two.sided = <? getRK ("two_sided"); ?>)
		results[i, 'G'] <- t$statistic["G"]
		results[i, 'U'] <- t$statistic["U"]
		results[i, 'p-value'] <- t$p.value
		results[i, 'Alternative Hypothesis']<- rk.describe.alternative (t)
<?	if (getRK_val ("descriptives")) { ?>
		results[i, 'Mean'] <- mean (var)
		results[i, 'Standard Deviation'] <- sd (var)
		results[i, 'Median'] <- median (var)
		results[i, 'Minimum'] <- min (var)
		results[i, 'Maximum'] <- max (var)
<?	} ?>
		NA				# no error
	}, error=function (e) e$message)	# catch any errors
}
if (all (is.na (results$'Error'))) results$'Error' <- NULL
<?
}

function printout () {
?>
rk.header ("Grubbs tests for one or two outliers in data sample",
	parameters=list ("Type", "<? getRK ("type"); ?>", "Opposite", "<? getRK ("opposite"); ?>", "two-sided", "<? getRK ("two_sided"); ?>"))
rk.results (results)
<?
}

?>
