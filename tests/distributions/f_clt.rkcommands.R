local({
## Print result
# parameters:
df1 <- 5.0; df2 <- 5.0; ncp <- 0.0;
# generate the entire data:
data <- matrix(rf(n=10000, df1=df1, df2=df2, ncp=ncp), nrow=10);
# get the sample averages:
avg <- colMeans(data);
dist.hist <- hist(avg, plot=FALSE, breaks="Sturges");
rk.graph.on ()
try ({
	plot(dist.hist, freq=FALSE, lty="solid", density=-1, xlab="Sample Averages", main="F")
})
rk.graph.off ()
})
