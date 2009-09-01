<?
function preprocess () { ?>
require (tseries)
<?
}

function calculate () {
	$vars = "substitute (" . str_replace ("\n", "), substitute (", trim (getRK_val ("x"))) . ")";

?>
objects <- list (<? echo ($vars); ?>)
results <- data.frame ('Variable Name'=rep (NA, length (objects)), check.names=FALSE)
for (i in 1:length (objects)) {
	results[i, 'Variable Name'] <- rk.get.description (objects[[i]], is.substitute=TRUE)
	var <- eval (objects[[i]], envir=globalenv ())
<?	if (getRK_val ("length")) { ?>
	results[i, 'Length'] <- length (var)
	results[i, 'NAs'] <- sum (is.na(var))

<?	}
	if (getRK_val ("narm")) { ?>
	var <- var[!is.na (var)] 	# remove NAs
<?	} ?>
	try ({
		test <- kpss.test (var, null = "<? getRK ("null"); ?>", lshort = <? getRK ("lshort"); ?>)
		results[i, 'KPSS <? getRK ("null") ?>'] <- test$statistic
		results[i, 'Truncation lag parameter'] <- test$parameter
		results[i, 'p-value'] <- test$p.value
	})
}
<?
}

function printout () {
?>
rk.header ("KPSS Test for Level Stationarity",
	parameters=list ("null hypothesis"="<? getRK ("null"); ?>", "version of truncation lag parameter"="<? if (getRK_val ("lshort") == "TRUE") echo ("short"); else echo ("long"); ?>"))

rk.results (results)
<?
}
?>
