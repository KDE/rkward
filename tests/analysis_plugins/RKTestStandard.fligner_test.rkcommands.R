local({
## Prepare
## Compute
result <- fligner.test (list (women[["weight"]], women[["height"]], test50z, test50y, test50x, test10z, test10y, test10x))
## Print result
names <- rk.get.description (women[["weight"]], women[["height"]], test50z, test50y, test50x, test10z, test10y, test10x)

rk.header (result$method)

rk.results (list (
	'Variables'=names,
	'Fligner-Killeen:med X^2 test statistic'=result$statistic,
	'df'=result$parameter,
	'p-value'=result$p.value))
})
.rk.rerun.plugin.link(plugin="rkward::fligner_test", settings="x.available=women[[\\\"weight\\\"]]\\nwomen[[\\\"height\\\"]]\\ntest50z\\ntest50y\\ntest50x\\ntest10z\\ntest10y\\ntest10x", label="Run again")
.rk.make.hr()
