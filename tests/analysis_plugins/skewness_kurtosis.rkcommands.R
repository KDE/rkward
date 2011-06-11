local({
## Prepare
require(moments)
## Compute
vars <- rk.list (women[["weight"]], women[["height"]], warpbreaks[["breaks"]])
results <- data.frame ('Variable Name'=I(names (vars)), check.names=FALSE)
for (i in 1:length (vars)) {
	var <- vars[[i]]
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
