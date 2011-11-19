local({
## Prepare
require(eRm)
## Print result
rk.header("Andersen's LR test")

rk.graph.on()
lr.res <- LRtest(estimates.pcm, se=TRUE)
try(plotGOF(lr.res, conf=list(), ctrline=list()))
rk.graph.off()
})
