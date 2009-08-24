local({
## Prepare
require(moments)
## Compute

vars <- list (substitute (warpbreaks[["breaks"]]), substitute (test50z), substitute (test10x))
results <- data.frame ('Variable Name'=rep (NA, length (vars)), check.names=FALSE)

for (i in 1:length(vars)) {
	results[i, 'Variable Name'] <- rk.get.description (vars[[i]], is.substitute=TRUE)
	var <- eval (vars[[i]], envir=globalenv ())
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
