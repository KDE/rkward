local({
## Prepare
require(exactRankTests)

names <- rk.get.description (test50x, test10y)
## Compute
result <- ansari.exact (test50x, test10y, alternative = "two.sided", exact=TRUE, conf.int = TRUE)

## Print result
rk.header (result$method,
	parameters=list ("Comparing", paste (names[1], "against", names[2]),
	'H1', rk.describe.alternative (result),
	"Compute exact p-value", "yes",
	"Confidence Level", "0.95" ))

rk.results (list (
	'Variable Names'=names,
	'statistic'=result$statistic,
	'null.value'=result$null.value,
	p=result$p.value,
	'confidence interval percent'=(100 * attr(result$conf.int, "conf.level")),
	'confidence interval of difference'=result$conf.int,
	'estimate of the ratio of scales'=result$estimate))
})
.rk.rerun.plugin.link(plugin="rkward::ansari_bradley_exact_test", settings="alternative.string=two.sided\nconfint.state=TRUE\nconflevel.real=0.95\nexact.string=yes\nx.available=test50x\ny.available=test10y", label="Run again")
.rk.make.hr()
local({
## Prepare
require(exactRankTests)

names <- rk.get.description (test50x, test50y)
## Compute
result <- ansari.exact (test50x, test50y, alternative = "less", conf.int = FALSE)

## Print result
rk.header (result$method,
	parameters=list ("Comparing", paste (names[1], "against", names[2]),
	'H1', rk.describe.alternative (result),
	"Compute exact p-value", "automatic"))

rk.results (list (
	'Variable Names'=names,
	'statistic'=result$statistic,
	'null.value'=result$null.value,
	p=result$p.value))
})
.rk.rerun.plugin.link(plugin="rkward::ansari_bradley_exact_test", settings="alternative.string=less\nconfint.state=FALSE\nexact.string=automatic\nx.available=test50x\ny.available=test50y", label="Run again")
.rk.make.hr()
