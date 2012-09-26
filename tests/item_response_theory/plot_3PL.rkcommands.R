local({
## Prepare
require(ltm)
## Print result
rk.header("Birnbaum three parameter model plot")

rk.graph.on()
try({
	plot(estimates.3pl, type="IIC", items=0)
})
rk.graph.off()
})
