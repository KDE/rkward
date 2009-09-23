local({
## Prepare
require(nortest)
## Compute
vars <- list (substitute (rock[["shape"]]), substitute (rock[["perm"]]))
results <- data.frame ('Variable Name'=rep (NA, length (vars)), check.names=FALSE)
for (i in 1:length (vars)) {
	results[i, 'Variable Name'] <- rk.get.description (vars[[i]], is.substitute=TRUE)
	var <- eval (vars[[i]], envir=globalenv ())
	results[i, 'Length'] <- length (var)
	results[i, 'NAs'] <- sum (is.na(var))
	try ({
		test <- ad.test (var)
		results[i, 'Statistic'] <- paste (names (test$statistic), test$statistic, sep=" = ")
		results[i, 'p-value'] <- test$p.value
	})
}
## Print result
rk.header ("Anderson-Darling Normality Test")

rk.results (results)
})
.rk.rerun.plugin.link(plugin="rkward::ad_test", settings="length.state=1\nx.available=rock[[\\\"shape\\\"]]\\nrock[[\\\"perm\\\"]]", label="Run again")
.rk.make.hr()
