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
	var <- na.omit(eval (vars[[i]], envir=globalenv ()))
	# var_w_na is similiar to var but NAs are not removed
	var_w_na <- eval (vars[[i]], envir=globalenv ())
	results[i, 'Error'] <- tryCatch ({
		# This is the core of the calculation
		t <- chisq.out.test (var, opposite = <? getRK ("opposite"); ?>, variance = var (var))
		results[i, 'X-squared'] <- t$statistic
		results[i, 'p-value'] <- t$p.value
		results[i, 'Alternative Hypothesis']<- rk.describe.alternative (t)
		results[i, 'Variance'] <- var (var)
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
		results[i, 'NAs'] <- length (which(is.na(var_w_na)))
<? 	} ?>
		NA				# no error
	}, error=function (e) e$message)	# catch any errors
}
if (all (is.na (results$'Error'))) results$'Error' <- NULL
<?
}

function printout () {
?>
rk.header ("Chi-squared test for outlier",
	parameters=list ("Opposite", "<? getRK ("opposite"); ?>"))
rk.results (results)
<?
}

?>
