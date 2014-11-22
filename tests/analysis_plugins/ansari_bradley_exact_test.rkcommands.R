local({
## Prepare
require(exactRankTests)

names <- rk.get.description (test50x, test10y)
## Compute
result <- ansari.exact (test50x, test10y, alternative = "two.sided", exact=TRUE, conf.int = TRUE)

## Print result
rk.header (result$method, parameters=list("compare"=names[1],
	"against"=names[2],
	"H1"=rk.describe.alternative(result),
	"compute exact p-value"="Always",
	"confidence level"="0.95"))

rk.results (list (
	"Variable Names"=names,
	"statistic"=result$statistic,
	"null.value"=result$null.value,
	p=result$p.value,
	"confidence interval percent"=(100 * attr(result$conf.int, "conf.level")),
	"confidence interval of difference"=result$conf.int,
	"estimate of the ratio of scales"=result$estimate))
})
local({
## Prepare
require(exactRankTests)

names <- rk.get.description (test50x, test50y)
## Compute
result <- ansari.exact (test50x, test50y, alternative = "less", conf.int = FALSE)

## Print result
rk.header (result$method, parameters=list("compare"=names[1],
	"against"=names[2],
	"H1"=rk.describe.alternative(result),
	"compute exact p-value"="Automatic"))

rk.results (list (
	"Variable Names"=names,
	"statistic"=result$statistic,
	"null.value"=result$null.value,
	p=result$p.value))
})
