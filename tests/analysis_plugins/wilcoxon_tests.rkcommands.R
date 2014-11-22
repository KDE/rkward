local({
## Prepare
	names <- rk.get.description (test50x)
## Compute
	wcox.result <- wilcox.test(
		x=test50x,
		exact=TRUE,
		conf.int=TRUE
	)

## Print result
rk.header (wcox.result$method, parameters=list("Comparing"=names[1],
	"H1"=rk.describe.alternative (wcox.result),
	"Continuity correction in normal approximation for p-value"="FALSE",
	"Compute exact p-value"="Always",
	"Location (shift) to test against (mu)"="0.00"))
rk.results (list (
	"Variable Names"=names,
	"Statistic"=wcox.result$statistic,
	"Location Shift"=wcox.result$null.value,
	"Hypothesis"=wcox.result$alternative,
	p=wcox.result$p.value,
	"Confidence interval percent"=(100 * attr(wcox.result$conf.int, "conf.level")),
	"Confidence interval of difference"=wcox.result$conf.int,
	"Difference in Location"=wcox.result$estimate))
})
local({
## Prepare
	names <- rk.get.description (test50x, test50y)
## Compute
	wcox.result <- wilcox.test(
		x=test50x,
		y=test50y,
		alternative="less",
		paired=TRUE,
		correct=TRUE
	)

## Print result
rk.header (wcox.result$method, parameters=list("Comparing"=names[1],
	"against"=names[2],
	"H1"=rk.describe.alternative (wcox.result),
	"Continuity correction in normal approximation for p-value"="TRUE",
	"Compute exact p-value"="Automatic",
	"Paired test"="yes",
	"Location (shift) to test against (mu)"="0.00"))
rk.results (list (
	"Variable Names"=names,
	"Statistic"=wcox.result$statistic,
	"Location Shift"=wcox.result$null.value,
	"Hypothesis"=wcox.result$alternative,
	p=wcox.result$p.value))
})
local({
## Prepare
	require(exactRankTests)
	names <- rk.get.description (test50x)
## Compute
	wcox.result <- wilcox.exact(
		x=test50x,
		exact=TRUE,
		conf.int=TRUE
	)

## Print result
rk.header (wcox.result$method, parameters=list("Comparing"=names[1],
	"H1"=rk.describe.alternative (wcox.result),
	"Continuity correction in normal approximation for p-value"="FALSE",
	"Compute exact p-value"="Always",
	"Location (shift) to test against (mu)"="0.00"))
rk.results (list (
	"Variable Names"=names,
	"Statistic"=wcox.result$statistic,
	"Location Shift"=wcox.result$null.value,
	"Hypothesis"=wcox.result$alternative,
	p=wcox.result$p.value,
	"Confidence interval percent"=(100 * attr(wcox.result$conf.int, "conf.level")),
	"Confidence interval of difference"=wcox.result$conf.int,
	"Difference in Location"=wcox.result$estimate))
})
local({
## Prepare
	require(exactRankTests)
	names <- rk.get.description (test50x, test50y)
## Compute
	wcox.result <- wilcox.exact(
		x=test50x,
		y=test50y,
		alternative="less",
		paired=TRUE,
		correct=TRUE
	)

## Print result
rk.header (wcox.result$method, parameters=list("Comparing"=names[1],
	"against"=names[2],
	"H1"=rk.describe.alternative (wcox.result),
	"Continuity correction in normal approximation for p-value"="TRUE",
	"Compute exact p-value"="Automatic",
	"Paired test"="yes",
	"Location (shift) to test against (mu)"="0.00"))
rk.results (list (
	"Variable Names"=names,
	"Statistic"=wcox.result$statistic,
	"Location Shift"=wcox.result$null.value,
	"Hypothesis"=wcox.result$alternative,
	p=wcox.result$p.value))
})
