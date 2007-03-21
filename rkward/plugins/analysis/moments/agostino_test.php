<?
function preprocess () { ?>
require(moments)
<?
}

function calculate () {
	global $alternative;

	$vars = "substitute (" . str_replace ("\n", "), substitute (", trim (getRK_val ("x"))) . ")";	$alternative = getRK_val ("alternative");
?>

vars <- list (<? echo ($vars); ?>)
results <- data.frame ('Variable Name'=rep (NA, length (vars)), check.names=FALSE)

for (i in 1:length(vars)) {
	results[i, 'Variable Name'] <- rk.get.description (vars[[i]], is.substitute=TRUE)
	var <- eval (vars[[i]], envir=globalenv ())
	results[i, 'Error'] <- tryCatch ({
		# This is the core of the calculation
		t <- agostino.test (var, alternative = "<? echo ($alternative); ?>")
		results[i, 'skewness estimator (skew)'] <- t$statistic["skew"]
		results[i, 'transformation (z)'] <- t$statistic["z"]
		results[i, 'p-value'] <- t$p.value
<? 	if (getRK_val ("show_alternative")) { ?>
		results[i, 'Alternative Hypothesis'] <- rk.describe.alternative (t)
<?	} ?>
		NA				# no error
	}, error=function (e) e$message)	# catch any errors
<?	if (getRK_val ("length")) { ?>
	results[i, 'Length'] <- length (var)
<?	}
	if (getRK_val ("nacount")) { ?>
	results[i, 'NAs'] <- length (which(is.na(var)))
<? 	} ?>
}
if (all (is.na (results$'Error'))) results$'Error' <- NULL
<?
}

function printout () {
	global $alternative;
?>
rk.header ("D'Agostino test of skewness",
	parameters=list ("Alternative Hypothesis", "<? echo ($alternative); ?>"))
rk.results (results)
<?
}

?>
