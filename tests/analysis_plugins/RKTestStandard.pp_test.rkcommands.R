local({
## Prepare
## Compute
objects <- list (substitute (rock[["shape"]]), substitute (rock[["perm"]]), substitute (rock[["peri"]]), substitute (rock[["area"]]))
results <- data.frame ('Variable Name'=rep (NA, length (objects)), check.names=FALSE)
for (i in 1:length (objects)) {
	results[i, 'Variable Name'] <- rk.get.description (objects[[i]], is.substitute=TRUE)
	var <- eval (objects[[i]], envir=globalenv ())
	results[i, 'Length'] <- length (var)
	results[i, 'NAs'] <- sum (is.na(var))

	try ({
		test <- PP.test (var, lshort = FALSE)
		results[i, 'Dickey-Fuller'] <- test$statistic
		results[i, 'Truncation lag parameter'] <- test$parameter
		results[i, 'p-value'] <- test$p.value
	})
}
## Print result
rk.header ("Phillips-Perron Test for Unit Roots",
	parameters=list ("Truncation lag parameter short ('TRUE') or long ('FALSE')", "FALSE"))

rk.results (results)
})
.rk.rerun.plugin.link(plugin="rkward::PP_test", settings="length.state=1\nlshort.string=FALSE\nnarm.state=0\nx.available=rock[[\\\"shape\\\"]]\\nrock[[\\\"perm\\\"]]\\nrock[[\\\"peri\\\"]]\\nrock[[\\\"area\\\"]]", label="Run again")
.rk.make.hr()
