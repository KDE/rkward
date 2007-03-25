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
	var <- eval (vars[[i]], envir=globalenv ())
<?	} ?>

	results[i, 'Error'] <- tryCatch ({
		# This is the core of the calculation
		t <- outlier (var, opposite = <? getRK ("opposite"); ?>)
		results[i, 'Outlier'] <- t
<?	if (getRK_val ("mean")) { ?>
		results[i, 'Mean'] <- mean (var)
<?	}
	if (getRK_val ("sd")) { ?>
		results[i, 'Standard Deviation'] <-  sd (var)
<?	}
	if (getRK_val ("median")) { ?>
		results[i, 'Median'] <- median (var)
<?	}
	if (getRK_val ("min")) { ?>
		results[i, 'Minimum'] <- min (var)
<?	}
	if (getRK_val ("max")) { ?>
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
rk.header ("Outlier",
	parameters=list ("Opposite", "<? getRK ("opposite"); ?>"))
rk.results (results)
<?
}

?>
