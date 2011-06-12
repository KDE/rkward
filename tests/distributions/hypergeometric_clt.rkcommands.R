local({
## Prepare
## Compute
## Print result
# parameters:
m <- 12; n <- 12; k <- 15;
# mean and variances of the distribution of sample averages:
avg.exp <- k*m/(m+n);
avg.var <- (k*m*n*(m+n-k)/((m+n)^2*(m+n-1)))/10;
# generate the entire data:
data <- matrix(rhyper(nn=10000, m=m, n=n, k=k), nrow=10);
# get the sample averages:
avg <- colMeans(data);
# generate random normal samples:
normX <- seq(from=min(avg), to=max(avg), length=1000);
normY <- dnorm (normX, mean = avg.exp, sd = sqrt(avg.var));
dist.hist <- hist(avg, plot=FALSE, breaks="Sturges");
# calculate the ylims appropriately:
ylim <- c(0,max(c(dist.hist$density, normY)));
rk.graph.on ()
try ({
	plot(dist.hist, ylim=ylim, freq=FALSE, lty="solid", density=-1, xlab="Sample Averages", main="Hypergeometric")
	lines (x=normX, y=normY, type="l", col="red")
})
rk.graph.off ()
})
