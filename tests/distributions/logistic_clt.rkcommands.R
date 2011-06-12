local({
## Prepare
## Compute
## Print result
# parameters:
loc <- 0.0; scale <- 3.0;
# mean and variances of the distribution of sample averages:
avg.exp <- loc;
avg.var <- ((pi^2/3)*scale^2)/10;
# generate the entire data:
data <- matrix(rlogis(n=10000, location=loc, scale=scale), nrow=10);
# get the sample averages:
avg <- colMeans(data);
# generate random normal samples:
normX <- seq(from=min(avg), to=max(avg), length=1000);
normY <- dnorm (normX, mean = avg.exp, sd = sqrt(avg.var));
dist.hist <- hist(avg, plot=FALSE, breaks=seq (floor (min (avg, na.rm=TRUE))-0.5, ceiling (max (avg, na.rm=TRUE))+0.5), right=FALSE);
# calculate the ylims appropriately:
ylim <- c(0,max(c(dist.hist$density, normY)));
rk.graph.on ()
try ({
	plot(dist.hist, ylim=ylim, freq=FALSE, labels=TRUE, lty="solid", density=-1, xlab="Sample Averages", main="Logistic")
	lines (x=normX, y=normY, type="l", col="red")
})
rk.graph.off ()
})
