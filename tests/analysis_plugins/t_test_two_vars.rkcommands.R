local({
## Prepare
names <- rk.get.description (test50x, test50y)
## Compute
result <- t.test (test50x, test50y, alternative="two.sided")
## Print result
rk.header (result$method, 
	parameters=list ("Comparing", paste (names[1], "against", names[2]),
	"H1", rk.describe.alternative (result),
	"Equal variances", "not assumed"))

rk.results (list (
	'Variable Name'=names,
	'estimated mean'=result$estimate,
	'degrees of freedom'=result$parameter,
	t=result$statistic,
	p=result$p.value,
	'confidence interval percent'=(100 * attr(result$conf.int, "conf.level")),
	'confidence interval of difference'=result$conf.int ))
})
local({
## Prepare
names <- rk.get.description (test10y, test10z)
## Compute
result <- t.test (test10y, test10z, alternative="less", paired=TRUE, conf.level=0.99)
## Print result
rk.header (result$method, 
	parameters=list ("Comparing", paste (names[1], "against", names[2]),
	"H1", rk.describe.alternative (result)))

rk.results (list (
	'Variable Name'=names,
	'estimated mean'=result$estimate,
	'degrees of freedom'=result$parameter,
	t=result$statistic,
	p=result$p.value,
	'confidence interval percent'=(100 * attr(result$conf.int, "conf.level")),
	'confidence interval of difference'=result$conf.int ))
})
