local({
## Prepare
require(eRm)
## Compute
## Print result
rk.header("Partial credit model plot")

rk.graph.on()
try(plotICC(estimates.pcm, item.subset=c(3:6), mplot=TRUE, ask=FALSE))
rk.graph.off()
})
