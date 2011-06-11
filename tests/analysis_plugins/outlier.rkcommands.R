local({
## Prepare
require(outliers)
## Compute
vars <- rk.list (warpbreaks[["breaks"]], test50z)
results <- data.frame ('Variable Name'=I(names (vars)), check.names=FALSE)
for (i in 1:length (vars)) {
	var <- vars[[i]]

	results[i, 'Length'] <- length (var)
	results[i, 'NAs'] <- sum (is.na(var))

	var <- na.omit (var) 	# omit NAs for all further calculations

	results[i, 'Error'] <- tryCatch ({
		# This is the core of the calculation
		t <- outlier (var, opposite = FALSE)
		results[i, 'Outlier'] <- t
		NA				# no error
	}, error=function (e) e$message)	# catch any errors
}
if (all (is.na (results$'Error'))) results$'Error' <- NULL
## Print result
rk.header ("Find potential outlier",
	parameters=list ("Opposite", "FALSE"))
rk.results (results)
})
.rk.rerun.plugin.link(plugin="rkward::outlier", settings="descriptives.state=0\nlength.state=1\nopposite.state=FALSE\nx.available=warpbreaks[[\\\"breaks\\\"]]\\ntest50z", label="Run again")
.rk.make.hr()
local({
## Prepare
require(outliers)
## Compute
vars <- rk.list (warpbreaks[["breaks"]], test50z)
results <- data.frame ('Variable Name'=I(names (vars)), check.names=FALSE)
for (i in 1:length (vars)) {
	var <- na.omit (vars[[i]])

	results[i, 'Error'] <- tryCatch ({
		# This is the core of the calculation
		t <- outlier (var, opposite = TRUE)
		results[i, 'Outlier'] <- t
		results[i, 'Mean'] <- mean (var)
		results[i, 'Standard Deviation'] <- sd (var)
		results[i, 'Median'] <- median (var)
		results[i, 'Minimum'] <- min (var)
		results[i, 'Maximum'] <- max (var)
		NA				# no error
	}, error=function (e) e$message)	# catch any errors
}
if (all (is.na (results$'Error'))) results$'Error' <- NULL
## Print result
rk.header ("Find potential outlier",
	parameters=list ("Opposite", "TRUE"))
rk.results (results)
})
.rk.rerun.plugin.link(plugin="rkward::outlier", settings="descriptives.state=1\nlength.state=0\nopposite.state=TRUE\nx.available=warpbreaks[[\\\"breaks\\\"]]\\ntest50z", label="Run again")
.rk.make.hr()
