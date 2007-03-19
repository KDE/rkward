<?
function preprocess () {
?>
require(moments)
<?}

function calculate () {
	$vars = "substitute (" . str_replace ("\n", "), substitute (", trim (getRK_val ("x"))) . ")";

?>

vars <- list (<? echo ($vars); ?>)
results <- data.frame ('Variable Name'=rep (NA, length (vars)), check.names=FALSE)

for (i in 1:length(vars)) {
	results[i, 'Variable Name'] <- rk.get.description (vars[[i]], is.substitute=TRUE)
	var <- eval (vars[[i]], envir=globalenv ())
	results[i, 'Error'] <- tryCatch ({
		t <- bonett.test (var, alternative = "<? getRK ("alternative"); ?>")
		results[i, 'Kurtosis estimator (tau)'] <- t$statistic["tau"]
		results[i, 'Transformation (z)'] <- t$statistic["z"]
		results[i, 'p-value'] <- t$p.value
		results[i, 'Alternative Hypothesis']<- rk.describe.alternative (t)
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
rk.header ("Bonett-Seier test of Geary's kurtosis",
	parameters=list ("Alternative Hypothesis", "<? getRK ("alternative"); ?>"))
rk.results (results)
<?
}
?>