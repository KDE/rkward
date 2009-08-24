local({
## Prepare
require(moments)
## Compute

vars <- list (substitute (test50z), substitute (test50y), substitute (test50x), substitute (test10z), substitute (test10y), substitute (test10x))
results <- data.frame ('Variable Name'=rep (NA, length (vars)), check.names=FALSE)

for (i in 1:length(vars)) {
	results[i, 'Variable Name'] <- rk.get.description (vars[[i]], is.substitute=TRUE)
	var <- eval (vars[[i]], envir=globalenv ())
	results[i, 'Error'] <- tryCatch ({
		t <- bonett.test (var, alternative = "two.sided")
		results[i, 'Kurtosis estimator (tau)'] <- t$statistic["tau"]
		results[i, 'Transformation (z)'] <- t$statistic["z"]
		results[i, 'p-value'] <- t$p.value
		results[i, 'Alternative Hypothesis'] <- rk.describe.alternative (t)
		NA				# no error
	}, error=function (e) e$message)	# catch any errors
	results[i, 'Length'] <- length (var)
	results[i, 'NAs'] <- length (which(is.na(var)))
}
if (all (is.na (results$'Error'))) results$'Error' <- NULL
## Print result
rk.header ("Bonett-Seier test of Geary's kurtosis",
	parameters=list ("Alternative Hypothesis", "two.sided"))
rk.results (results)
})
.rk.rerun.plugin.link(plugin="rkward::bonett_test", settings="alternative.string=two.sided\nlength.state=1\nshow_alternative.state=1\nx.available=test50z\\ntest50y\\ntest50x\\ntest10z\\ntest10y\\ntest10x", label="Run again")
.rk.make.hr()
