local({
## Prepare
require(qcc)
require(xtable)
## Compute
## Print result
x <- swiss[["Catholic"]]
if (!is.numeric (x)) {
	warning ("Data may not be numeric, but proceeding as requested.\nDid you forget to check the tabulate option?")
}

rk.header ("Pareto chart")

rk.graph.on ()
try ({
	descriptives <- pareto.chart(x, main="swiss[[\"Catholic\"]]")
	rk.results(xtable(descriptives))
})
rk.graph.off ()
})
