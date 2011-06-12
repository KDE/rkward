local({
## Prepare
## Compute
## Print result
# parameters:
size <- 11; prob <- 0.50;
# mean and variances of the distribution of sample averages:
avg.exp <- size*prob;
avg.var <- (size*prob*(1-prob))/1;
# generate the entire data:
data <- matrix(rbinom(n=1000, size=size, prob=prob), nrow=1);
# get the sample averages:
avg <- colMeans(data);
# generate random normal samples:
normX <- seq(from=min(avg), to=max(avg), length=1000);
normY <- dnorm (normX, mean = avg.exp, sd = sqrt(avg.var));
dist.hist <- hist(avg, plot=FALSE, breaks=seq (floor (min (avg, na.rm=TRUE))-0.5, ceiling (max (avg, na.rm=TRUE))+0.5));
# calculate the ylims appropriately:
ylim <- c(0,max(c(dist.hist$density, normY)));
rk.graph.on ()
try ({
	plot(dist.hist, ylim=ylim, freq=FALSE, lty="solid", density=-1, xlab="Sample Averages", main="Binomial")
	lines (x=normX, y=normY, type="l", col="red")
})
rk.graph.off ()
})
