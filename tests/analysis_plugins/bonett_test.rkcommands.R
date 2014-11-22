local({
## Prepare
require(moments)
## Compute
vars <- rk.list (test50z, test50y, test50x, test10z, test10y, test10x)
results <- data.frame ('Variable Name'=I(names (vars)), check.names=FALSE)
for (i in 1:length (vars)) {
	var <- vars[[i]]
	results[i, "Error"] <- tryCatch ({
		# This is the core of the calculation
		t <- bonett.test (var, alternative = "two.sided")
		results[i, 'Kurtosis estimator (tau)'] <- t$statistic["tau"]
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
rk.header ("Bonett-Seier test of Geary's kurtosis", parameters=list("Test hypothesis (H1)"="Two-sided"))
rk.results (results)
})
