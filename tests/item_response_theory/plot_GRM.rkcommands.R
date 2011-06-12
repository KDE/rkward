local({
## Prepare
require(ltm)
## Compute
## Print result
rk.header("Graded response model plot")

rk.graph.on()
try(plot(estimates.grm, type="ICC", items=c(6)))
rk.graph.off()
})
