local({
## Prepare
require (tseries)
## Compute
vars <- rk.list (test50x, test50y, test50z)
results <- data.frame ('Variable Name'=I(names (vars)), check.names=FALSE)
for (i in 1:length (vars)) {
	var <- vars[[i]]
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
