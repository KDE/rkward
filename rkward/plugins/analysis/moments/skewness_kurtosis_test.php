<?
function preprocess () { ?>
require(moments)
<?
}

function calculate () {
	$vars = "substitute (" . str_replace ("\n", "), substitute (", trim (getRK_val ("x"))) . ")";
	if (getRK_val ("narm")) $narm = ", na.rm=TRUE";
	else $narm = ", na.rm=FALSE"

?>
objects <- list (<? echo ($vars); ?>)
results <- data.frame ('Variable Name'=rep (NA, length (objects)), check.names=FALSE)
for (i in 1:length (objects)) {
	var <- eval (objects[[i]], envir=globalenv ())
	results[i, 'Variable Name'] <- rk.get.description (objects[[i]], is.substitute=TRUE)

	try ({
<?
	if (getRK_val ("skewness")) { ?>
		results[i, 'Skewness'] <- skewness (var<? echo ($narm); ?>)
<?	}
	if (getRK_val ("kurtosis")) { ?>
		results[i, 'Kurtosis'] <- kurtosis (var<? echo ($narm); ?>)
		results[i, 'Excess Kurtosis'] <- results[i, 'Kurtosis'] - 3
<?	}
	if (getRK_val ("geary")) { ?>
		results[i, 'Geary Kurtosis'] <- geary (var<? echo ($narm); ?>)
<?	} ?>
	})
<?	if (getRK_val ("length")) { ?>

	results[i, 'Length'] <- length (var)
	results[i, 'NAs'] <- sum (is.na(var))
<?	} ?>
}
<?
}
function printout () {
?>
rk.header ("Skewness and Kurtosis")
rk.results (results)
<?
}
?>
