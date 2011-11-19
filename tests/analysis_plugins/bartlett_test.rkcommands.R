local({
## Compute
result <- bartlett.test (list (warpbreaks[["breaks"]], test50z, test50y, test50x, test10z, test10y, test10x))
## Print result
names <- rk.get.description (warpbreaks[["breaks"]], test50z, test50y, test50x, test10z, test10y, test10x)

rk.header (result$method)

rk.results (list (
	'Variables'=names,
	'Bartlett s K-squared'=result$statistic,
	'df'=result$parameter,
	'p-value'=result$p.value))
})
