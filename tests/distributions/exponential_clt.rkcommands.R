local({
## Prepare
## Compute
## Print result
# parameters:
rate <- 1.0;
# mean and variances of the distribution of sample averages:
avg.exp <- 1/rate;
avg.var <- (1/(rate^2))/10;
# generate the entire data:
data <- matrix(rexp(n=10000, rate=rate), nrow=10);
# get the sample averages:
avg <- colMeans(data);
# normalise the variables:
avg <- (avg - avg.exp)/sqrt(avg.var);
# generate random normal samples:
normX <- seq(from=min(avg), to=max(avg), length=1000);
normY <- dnorm (normX);
dist.hist <- hist(avg, plot=FALSE, breaks="Sturges");
# calculate the ylims appropriately:
ylim <- c(0,max(c(dist.hist$density, normY)));
rk.graph.on ()
try ({
	plot(dist.hist, ylim=ylim, freq=FALSE, lty="solid", density=-1, xlab="Sample Averages", main="Exponential")
	lines (x=normX, y=normY, type="h", col="grey4")
})
rk.graph.off ()
})
