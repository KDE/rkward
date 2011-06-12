local({
## Prepare
require(eRm)
## Compute
## Print result
rk.header("Rating scale model plot")

rk.graph.on()
try(plotICC(estimates.rsm, item.subset=c(1:3), mplot=TRUE, ask=FALSE))
rk.graph.off()
})
