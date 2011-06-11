local({
## Prepare
require(tseries)
## Compute
vars <- rk.list (rock[["shape"]], rock[["perm"]])
results <- data.frame ('Variable Name'=I(names (vars)), check.names=FALSE)
for (i in 1:length (vars)) {
	var <- vars[[i]]
	results[i, 'Length'] <- length (var)
	results[i, 'NAs'] <- sum (is.na(var))
	var <- na.omit (var)
	try ({
		test <- jarque.bera.test (var)
		results[i, 'Statistic'] <- paste (names (test$statistic), test$statistic, sep=" = ")
		results[i, 'df'] <- test$parameter
		results[i, 'p-value'] <- test$p.value
	})
}
## Print result
rk.header ("Jarque-Bera Normality Test", parameters=list ("Exclude NAs", "YES"))
rk.results (results)
})
.rk.rerun.plugin.link(plugin="rkward::jb_test", settings="excludenas.state=1\nlength.state=1\nx.available=rock[[\\\"shape\\\"]]\\nrock[[\\\"perm\\\"]]", label="Run again")
.rk.make.hr()
