local({
## Prepare
## Compute
result <- mood.test (test50z, test50x, alternative = "two.sided")
## Print result
names <- rk.get.description (test50z, test50x)

rk.header (result$method,
	parameters=list ("Alternative Hypothesis", rk.describe.alternative (result)))

rk.results (list (
	'Variables'=names,
	'Z'=result$statistic,
	'p-value'=result$p.value))
})
