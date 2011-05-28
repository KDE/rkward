local({
## Prepare
require(moments)
## Compute
vars <- rk.list (warpbreaks[["breaks"]], test50z, test10x)
results <- data.frame ('Variable Name'=I(names (vars)), check.names=FALSE)
for (i in 1:length (vars)) {
	var <- vars[[i]]
	results[i, 'Error'] <- tryCatch ({
		# This is the core of the calculation
		t <- agostino.test (var, alternative = "two.sided")
		results[i, 'skewness estimator (skew)'] <- t$statistic["skew"]
		results[i, 'transformation (z)'] <- t$statistic["z"]
		results[i, 'p-value'] <- t$p.value
		results[i, 'Alternative Hypothesis'] <- rk.describe.alternative (t)
		NA				# no error
	}, error=function (e) e$message)	# catch any errors
	results[i, 'Length'] <- length (var)
	results[i, 'NAs'] <- sum (is.na(var))
}
if (all (is.na (results$'Error'))) results$'Error' <- NULL
## Print result
rk.header ("D'Agostino test of skewness",
	parameters=list ("Alternative Hypothesis", "two.sided"))
rk.results (results)
})
.rk.rerun.plugin.link(plugin="rkward::agostino_test", settings="alternative.string=two.sided\nlength.state=1\nshow_alternative.state=1\nx.available=warpbreaks[[\\\"breaks\\\"]]\\ntest50z\\ntest10x", label="Run again")
.rk.make.hr()
