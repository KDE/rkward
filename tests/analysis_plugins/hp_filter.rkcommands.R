local({
## Prepare
## Compute
## Print result
rk.header ("Hodrick-Prescott Filter", parameters=list("Lambda", 1600))
x <- co2
lambda <- 1600

if (any (is.na (x))) stop ("Missing values cannot be handled")

i <- diag(length(x))
trend <- solve(i + lambda * crossprod(diff(i, lag=1, d=2)), x) # The HP Filter itself. Thanks to Grant V. Farnsworth
cycle <- x - trend
if (is.ts(x)) {
	trend <- ts(trend,start(x),frequency=frequency(x))
	cycle <- ts(cycle,start(x),frequency=frequency(x))
}
.GlobalEnv$hptrend <- trend
.GlobalEnv$hpcycle <- cycle
rk.graph.on ()
try({
	par(mfrow=c(2,1),mar=c(2,4,2,2)+0.1)
	plot.ts(cbind(x, trend), ylab="co2, Trend", col=c("blue", "red"),lwd=c(1,1), plot.type="single")
	plot.ts(cycle, ylab="Cycle", col="green4", lwd=1)
})
rk.graph.off ()
})
