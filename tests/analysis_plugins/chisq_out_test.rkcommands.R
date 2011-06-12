local({
## Prepare
require(outliers)
## Compute
vars <- rk.list (rock[["shape"]], rock[["perm"]], rock[["peri"]], rock[["area"]])
results <- data.frame ('Variable Name'=I(names (vars)), check.names=FALSE)
for (i in 1:length (vars)) {
	var <- vars[[i]]

	results[i, 'Length'] <- length (var)
	results[i, 'NAs'] <- sum (is.na(var))

	var <- na.omit (var) 	# omit NAs for all further calculations

	results[i, 'Error'] <- tryCatch ({
		# This is the core of the calculation
		t <- chisq.out.test (var, opposite = FALSE)
		results[i, 'X-squared'] <- t$statistic
		results[i, 'p-value'] <- t$p.value
		results[i, 'Alternative Hypothesis']<- rk.describe.alternative (t)
		results[i, 'Variance'] <- var (var)
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
rk.header ("Chi-squared test for outlier",
	parameters=list ("Opposite", "FALSE"))
rk.results (results)
})
local({
## Prepare
require(outliers)
## Compute
vars <- rk.list (rock[["shape"]], rock[["perm"]], rock[["peri"]], rock[["area"]])
results <- data.frame ('Variable Name'=I(names (vars)), check.names=FALSE)
for (i in 1:length (vars)) {
	var <- na.omit (vars[[i]])

	results[i, 'Error'] <- tryCatch ({
		# This is the core of the calculation
		t <- chisq.out.test (var, opposite = TRUE)
		results[i, 'X-squared'] <- t$statistic
		results[i, 'p-value'] <- t$p.value
		results[i, 'Alternative Hypothesis']<- rk.describe.alternative (t)
		results[i, 'Variance'] <- var (var)
		NA				# no error
	}, error=function (e) e$message)	# catch any errors
}
if (all (is.na (results$'Error'))) results$'Error' <- NULL
## Print result
rk.header ("Chi-squared test for outlier",
	parameters=list ("Opposite", "TRUE"))
rk.results (results)
})
