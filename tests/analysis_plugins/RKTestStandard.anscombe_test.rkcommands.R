local({
## Prepare
require(moments)
## Compute
vars <- list (substitute (warpbreaks[["breaks"]]), substitute (test50z), substitute (test10y))
results <- data.frame ('Variable Name'=rep (NA, length (vars)), check.names=FALSE)

for (i in 1:length(vars)) {
	results[i, 'Variable Name'] <- rk.get.description (vars[[i]], is.substitute=TRUE)
	var <- eval (vars[[i]], envir=globalenv ())
	results[i, 'Error'] <- tryCatch ({
		t <- anscombe.test (var, alternative = "two.sided")
		results[i, 'Kurtosis estimator (tau)'] <- t$statistic["kurt"]
		results[i, 'Transformation (z)'] <- t$statistic["z"]
		results[i, 'p-value'] <- t$p.value
		results[i, 'Alternative Hypothesis'] <- rk.describe.alternative (t)
		NA				# no error
	}, error=function (e) e$message)	# catch any errors
	results[i, 'Length'] <- length (var)
	results[i, 'NAs'] <- sum (is.na(var))
}
if (all (is.na (results$'Error'))) results$'Error' <- NULL
## Print result
rk.header ("Anscombe-Glynn test of kurtosis",
	parameters=list ("Alternative Hypothesis", "two.sided"))
rk.results (results)
})
.rk.rerun.plugin.link(plugin="rkward::anscombe_test", settings="alternative.string=two.sided\nlength.state=1\nshow_alternative.state=1\nx.available=warpbreaks[[\\\"breaks\\\"]]\\ntest50z\\ntest10y", label="Run again")
.rk.make.hr()
