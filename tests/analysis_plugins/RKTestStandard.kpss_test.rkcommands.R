local({
## Prepare
require (tseries)
## Compute
objects <- list (substitute (test50x), substitute (test50y), substitute (test50z))
results <- data.frame ('Variable Name'=rep (NA, length (objects)), check.names=FALSE)
for (i in 1:length (objects)) {
	results[i, 'Variable Name'] <- rk.get.description (objects[[i]], is.substitute=TRUE)
	var <- eval (objects[[i]], envir=globalenv ())
	results[i, 'Length'] <- length (var)
	results[i, 'NAs'] <- sum (is.na(var))

	try ({
		test <- kpss.test (var, null = "Trend", lshort = FALSE)
		results[i, 'KPSS Trend'] <- test$statistic
		results[i, 'Truncation lag parameter'] <- test$parameter
		results[i, 'p-value'] <- test$p.value
	})
}
## Print result
rk.header ("KPSS Test for Level Stationarity",
	parameters=list ("null hypothesis"="Trend", "version of truncation lag parameter"="long"))
rk.results (results)
})
.rk.rerun.plugin.link(plugin="rkward::kpss_test", settings="length.state=1\nlshort.string=FALSE\nnarm.state=0\nnull.string=Trend\nx.available=test50x\\ntest50y\\ntest50z", label="Run again")
.rk.make.hr()
