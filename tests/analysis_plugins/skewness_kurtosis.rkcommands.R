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
