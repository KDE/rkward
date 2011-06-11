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
.rk.rerun.plugin.link(plugin="rkward::t_test_two_vars", settings="confint.state=1\nconflevel.real=0.95\nhypothesis.string=two.sided\npaired.state=0\nvarequal.state=0\nx.available=test50x\ny.available=test50y", label="Run again")
.rk.make.hr()
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
.rk.rerun.plugin.link(plugin="rkward::t_test_two_vars", settings="confint.state=1\nconflevel.real=0.99\nhypothesis.string=less\npaired.state=1\nx.available=test10y\ny.available=test10z", label="Run again")
.rk.make.hr()
