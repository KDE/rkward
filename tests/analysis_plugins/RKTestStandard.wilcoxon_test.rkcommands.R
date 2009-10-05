local({
## Prepare
names = rk.get.description (test50x)
## Compute
result <- wilcox.test (test50x, alternative = "two.sided", mu = 0.00, exact=TRUE, correct = FALSE, conf.int = TRUE)

## Print result
rk.header (result$method,
	parameters=list ("Comparing", paste (names, collapse=" against "),
	"H1", rk.describe.alternative (result),
	"Continuity correction in normal approximation for p-value", "FALSE",
	"Compute exact p-value", "yes", "Paired test", "FALSE",
	"mu", "0.00"))

rk.results (list (
	'Variable Names'=names,
	'statistic'=result$statistic,
	'Location Shift'=result$null.value,
	'Hypothesis'=result$alternative,
	p=result$p.value,
	'confidence interval percent'=(100 * attr(result$conf.int, "conf.level")),
	'confidence interval of difference'=result$conf.int,
	'Difference in Location' = result$estimate))
})
.rk.rerun.plugin.link(plugin="rkward::wilcoxon_test", settings="alternative.string=two.sided\nconfint.state=TRUE\nconflevel.real=0.95\ncorrect.state=FALSE\nexact.string=yes\nmu.real=0.00\nx.available=test50x\ny.available=", label="Run again")
.rk.make.hr()
local({
## Prepare
names = rk.get.description (test50x, test50y)
## Compute
result <- wilcox.test (test50x, test50y, alternative = "less", mu = 0.00, paired = TRUE, correct = TRUE, conf.int = FALSE)

## Print result
rk.header (result$method,
	parameters=list ("Comparing", paste (names, collapse=" against "),
	"H1", rk.describe.alternative (result),
	"Continuity correction in normal approximation for p-value", "TRUE",
	"Compute exact p-value", "automatic", "Paired test", "TRUE",
	"mu", "0.00"))

rk.results (list (
	'Variable Names'=names,
	'statistic'=result$statistic,
	'Location Shift'=result$null.value,
	'Hypothesis'=result$alternative,
	p=result$p.value))
})
.rk.rerun.plugin.link(plugin="rkward::wilcoxon_test", settings="alternative.string=less\nconfint.state=FALSE\ncorrect.state=TRUE\nexact.string=automatic\nmu.real=0.00\npaired.state=TRUE\nx.available=test50x\ny.available=test50y", label="Run again")
.rk.make.hr()
