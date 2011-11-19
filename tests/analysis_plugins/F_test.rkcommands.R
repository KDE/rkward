local({
## Compute
result <- var.test (test50z, test50y, alternative = "two.sided", ratio = 1.00)

## Print result
names <- rk.get.description (test50z, test50y)

rk.header (result$method,
	parameters=list ("Confidence Level", "0.95", "Alternative Hypothesis", rk.describe.alternative(result)))

rk.results (list (
	'Variables'=names,
	'F'=result$statistic["F"],
	'Numerator DF'=result$parameter["num df"],
	'Denominator DF'=result$parameter["denom df"],
	'p-value'=result$p.value,
	'Lower CI'=result$conf.int[1],
	'Upper CI'=result$conf.int[2],
	'ratio of variances'=result$estimate))
})
