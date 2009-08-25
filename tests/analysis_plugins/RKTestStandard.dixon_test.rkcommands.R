local({
## Prepare
require(outliers)
## Compute

vars <- list (substitute (women[["weight"]]), substitute (women[["height"]]))
results <- data.frame ('Variable Name'=rep (NA, length (vars)), check.names=FALSE)
for (i in 1:length(vars)) {
	results[i, 'Variable Name'] <- rk.get.description (vars[[i]], is.substitute=TRUE)
	var <- eval (vars[[i]], envir=globalenv ())

	results[i, 'Length'] <- length (var)
	results[i, 'NAs'] <- sum (is.na(var))

	var <- na.omit (var) 	# omit NAs for all further calculations

	results[i, 'Error'] <- tryCatch ({
		# This is the core of the calculation
		t <- dixon.test (var, type = 0, opposite = FALSE, two.sided = TRUE)
		results[i, 'Dixon Q-statistic'] <- t$statistic["Q"]
		results[i, 'p-value'] <- t$p.value
		results[i, 'Alternative Hypothesis']<- rk.describe.alternative (t)
		results[i, 'Mean'] <- mean (var)
		results[i, 'Standard Deviation'] <-  sd (var)
		results[i, 'Median'] <- median (var)
		results[i, 'Minimum'] <- min (var)
		results[i, 'Maximum'] <- max (var)
		NA				# no error
	}, error=function (e) e$message)	# catch any errors
}
if (all (is.na (results$'Error'))) results$'Error' <- NULL
## Print result
rk.header ("Dixon test for outlier",
	parameters=list ("Type", "0", "Opposite", "FALSE", "two-sided", "TRUE"))
rk.results (results)
})
.rk.rerun.plugin.link(plugin="rkward::dixon_test", settings="descriptives.state=1\nlength.state=1\nopposite.state=FALSE\ntwo_sided.state=TRUE\ntype.string=0\nx.available=women[[\\\"weight\\\"]]\\nwomen[[\\\"height\\\"]]", label="Run again")
.rk.make.hr()
local({
## Prepare
require(outliers)
## Compute

vars <- list (substitute (women[["weight"]]), substitute (women[["height"]]))
results <- data.frame ('Variable Name'=rep (NA, length (vars)), check.names=FALSE)
for (i in 1:length(vars)) {
	results[i, 'Variable Name'] <- rk.get.description (vars[[i]], is.substitute=TRUE)
	var <- na.omit (eval (vars[[i]], envir=globalenv ()))

	results[i, 'Error'] <- tryCatch ({
		# This is the core of the calculation
		t <- dixon.test (var, type = 0, opposite = TRUE, two.sided = FALSE)
		results[i, 'Dixon Q-statistic'] <- t$statistic["Q"]
		results[i, 'p-value'] <- t$p.value
		results[i, 'Alternative Hypothesis']<- rk.describe.alternative (t)
		NA				# no error
	}, error=function (e) e$message)	# catch any errors
}
if (all (is.na (results$'Error'))) results$'Error' <- NULL
## Print result
rk.header ("Dixon test for outlier",
	parameters=list ("Type", "0", "Opposite", "TRUE", "two-sided", "FALSE"))
rk.results (results)
})
.rk.rerun.plugin.link(plugin="rkward::dixon_test", settings="descriptives.state=0\nlength.state=0\nopposite.state=TRUE\ntwo_sided.state=FALSE\ntype.string=0\nx.available=women[[\\\"weight\\\"]]\\nwomen[[\\\"height\\\"]]", label="Run again")
.rk.make.hr()
