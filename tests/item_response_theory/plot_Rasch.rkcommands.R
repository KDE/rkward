local({
## Prepare
require(ltm)
## Print result
rk.header ("Rasch model plot")

rk.graph.on()
try({
	plot(estimates.rasch, type="ICC", legend=TRUE)
})
rk.graph.off()
})
