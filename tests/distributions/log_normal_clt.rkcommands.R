local({
## Print result
# parameters:
mean <- 0.0; sd <- 1.00;
# mean and variances of the distribution of sample averages:
avg.exp <- exp(mean+sd^2/2);
avg.var <- (exp(2*mean+sd^2)*(exp(sd^2)-1))/10;
# generate the entire data:
data <- matrix(rlnorm(n=10000, meanlog=mean, sdlog=sd), nrow=10);
# get the sample averages:
avg <- colMeans(data);
# generate random normal samples:
normX <- seq(from=min(avg), to=max(avg), length=1000);
normY <- dnorm (normX, mean = avg.exp, sd = sqrt(avg.var));
dist.hist <- hist(avg, plot=FALSE, breaks="Scott");
# calculate the ylims appropriately:
ylim <- c(0,max(c(dist.hist$density, normY)));
rk.graph.on ()
try ({
	plot(dist.hist, ylim=ylim, freq=FALSE, lty="solid", density=7, angle=45, border=FALSE, xlab="Sample Averages", main="Lognormal")
	lines (x=normX, y=normY, type="l", col="red")
})
rk.graph.off ()
})
