local({
## Prepare
require(moments)
## Compute
objects <- list (substitute (women[["weight"]]), substitute (women[["height"]]), substitute (warpbreaks[["breaks"]]))
results <- data.frame ('Variable Name'=rep (NA, length (objects)), check.names=FALSE)
for (i in 1:length (objects)) {
	var <- eval (objects[[i]], envir=globalenv ())
	results[i, 'Variable Name'] <- rk.get.description (objects[[i]], is.substitute=TRUE)

	try ({
		results[i, 'Skewness'] <- skewness (var, na.rm=TRUE)
		results[i, 'Kurtosis'] <- kurtosis (var, na.rm=TRUE)
		results[i, 'Excess Kurtosis'] <- results[i, 'Kurtosis'] - 3
		results[i, 'Geary Kurtosis'] <- geary (var, na.rm=TRUE)
	})

	results[i, 'Length'] <- length (var)
	results[i, 'NAs'] <- sum (is.na(var))
}
## Print result
rk.header ("Skewness and Kurtosis")
rk.results (results)
})
.rk.rerun.plugin.link(plugin="rkward::skewness_kurtosis", settings="geary.state=1\nkurtosis.state=1\nlength.state=1\nnarm.state=1\nskewness.state=1\nx.available=women[[\\\"weight\\\"]]\\nwomen[[\\\"height\\\"]]\\nwarpbreaks[[\\\"breaks\\\"]]", label="Run again")
.rk.make.hr()
