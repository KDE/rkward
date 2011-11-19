local({
## Prepare
require(ltm)
## Print result
rk.header("Two parameter logistic model plot")

rk.graph.on()
try(plot(estimates.2pl, type="ICC", items=c(1)))
rk.graph.off()
})
