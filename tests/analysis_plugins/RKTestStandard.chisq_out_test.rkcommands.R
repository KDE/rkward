local({
## Prepare
require(outliers)
## Compute

vars <- list (substitute (rock[["shape"]]), substitute (rock[["perm"]]), substitute (rock[["peri"]]), substitute (rock[["area"]]))
results <- data.frame ('Variable Name'=rep (NA, length (vars)), check.names=FALSE)
for (i in 1:length(vars)) {
	results[i, 'Variable Name'] <- rk.get.description (vars[[i]], is.substitute=TRUE)
	var <- eval (vars[[i]], envir=globalenv ())

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
.rk.rerun.plugin.link(plugin="rkward::chisq_out_test", settings="descriptives.state=1\nlength.state=1\nopposite.state=FALSE\nx.available=rock[[\\\"shape\\\"]]\\nrock[[\\\"perm\\\"]]\\nrock[[\\\"peri\\\"]]\\nrock[[\\\"area\\\"]]", label="Run again")
.rk.make.hr()
local({
## Prepare
require(outliers)
## Compute

vars <- list (substitute (rock[["shape"]]), substitute (rock[["perm"]]), substitute (rock[["peri"]]), substitute (rock[["area"]]))
results <- data.frame ('Variable Name'=rep (NA, length (vars)), check.names=FALSE)
for (i in 1:length(vars)) {
	results[i, 'Variable Name'] <- rk.get.description (vars[[i]], is.substitute=TRUE)
	var <- na.omit (eval (vars[[i]], envir=globalenv ()))

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
.rk.rerun.plugin.link(plugin="rkward::chisq_out_test", settings="descriptives.state=0\nlength.state=0\nopposite.state=TRUE\nx.available=rock[[\\\"shape\\\"]]\\nrock[[\\\"perm\\\"]]\\nrock[[\\\"peri\\\"]]\\nrock[[\\\"area\\\"]]", label="Run again")
.rk.make.hr()
