local({
## Prepare
require(moments)
## Compute
vars <- rk.list (warpbreaks[["breaks"]], test50z, test10y)
results <- data.frame ('Variable Name'=I(names (vars)), check.names=FALSE)
for (i in 1:length (vars)) {
	var <- vars[[i]]
	results[i, "Error"] <- tryCatch ({
		# This is the core of the calculation
		t <- anscombe.test (var, alternative = "two.sided")
		results[i, 'Kurtosis estimator (tau)'] <- t$statistic["kurt"]
		results[i, 'Transformation (z)'] <- t$statistic["z"]
		results[i, 'p-value'] <- t$p.value
		results[i, 'Alternative Hypothesis'] <- rk.describe.alternative (t)
		NA				# no error
	}, error=function (e) e$message)	# catch any errors
	results[i, "Length"] <- length (var)
	results[i, 'NAs'] <- sum (is.na(var))
}
if (all (is.na (results$"Error"))) results$"Error" <- NULL
## Print result
rk.header ("Anscombe-Glynn test of kurtosis", parameters=list("Test hypothesis (H1)"="Two-sided"))
rk.results (results)
})
