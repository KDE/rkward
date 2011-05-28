local({
## Prepare
require(outliers)
## Compute
vars <- rk.list (warpbreaks[["breaks"]], test10z)
results <- data.frame ('Variable Name'=I(names (vars)), check.names=FALSE)
for (i in 1:length (vars)) {
	var <- vars[[i]]

	results[i, 'Length'] <- length (var)
	results[i, 'NAs'] <- sum (is.na(var))

	var <- na.omit (var) 	# omit NAs for all further calculations

	results[i, 'Error'] <- tryCatch ({
		# This is the core of the calculation
		t <- grubbs.test (var, type = 10, opposite = FALSE, two.sided = TRUE)
		results[i, 'G'] <- t$statistic["G"]
		results[i, 'U'] <- t$statistic["U"]
		results[i, 'p-value'] <- t$p.value
		results[i, 'Alternative Hypothesis']<- rk.describe.alternative (t)
		NA				# no error
	}, error=function (e) e$message)	# catch any errors
}
if (all (is.na (results$'Error'))) results$'Error' <- NULL
## Print result
rk.header ("Grubbs tests for one or two outliers in data sample",
	parameters=list ("Type", "10", "Opposite", "FALSE", "two-sided", "TRUE"))
rk.results (results)
})
.rk.rerun.plugin.link(plugin="rkward::grubbs_test", settings="descriptives.state=0\nlength.state=1\nopposite.state=FALSE\ntwo_sided.state=TRUE\ntype.string=10\nx.available=warpbreaks[[\\\"breaks\\\"]]\\ntest10z", label="Run again")
.rk.make.hr()
local({
## Prepare
require(outliers)
## Compute
vars <- rk.list (warpbreaks[["breaks"]], test10z)
results <- data.frame ('Variable Name'=I(names (vars)), check.names=FALSE)
for (i in 1:length (vars)) {
	var <- vars[[i]]

	results[i, 'Length'] <- length (var)
	results[i, 'NAs'] <- sum (is.na(var))

	var <- na.omit (var) 	# omit NAs for all further calculations

	results[i, 'Error'] <- tryCatch ({
		# This is the core of the calculation
		t <- grubbs.test (var, type = 11, opposite = TRUE, two.sided = FALSE)
		results[i, 'G'] <- t$statistic["G"]
		results[i, 'U'] <- t$statistic["U"]
		results[i, 'p-value'] <- t$p.value
		results[i, 'Alternative Hypothesis']<- rk.describe.alternative (t)
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
rk.header ("Grubbs tests for one or two outliers in data sample",
	parameters=list ("Type", "11", "Opposite", "TRUE", "two-sided", "FALSE"))
rk.results (results)
})
.rk.rerun.plugin.link(plugin="rkward::grubbs_test", settings="descriptives.state=1\nlength.state=1\nopposite.state=TRUE\ntwo_sided.state=FALSE\ntype.string=11\nx.available=warpbreaks[[\\\"breaks\\\"]]\\ntest10z", label="Run again")
.rk.make.hr()
