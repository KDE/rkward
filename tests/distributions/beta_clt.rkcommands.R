local({
## Print result
# parameters:
shape1 <- 2.0; shape2 <- 2.0;
# mean and variances of the distribution of sample averages:
avg.exp <- shape1/(shape1 + shape2);
avg.var <- (shape1*shape2/((shape1+shape2)^2*(shape1+shape2+1)))/10;
# generate the entire data:
data <- matrix(rbeta(n=10000, shape1=shape1, shape2=shape2), nrow=10);
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
	plot(dist.hist, ylim=ylim, freq=FALSE, labels=TRUE, lty="solid", density=-1, col="azure", xlab="Sample Averages", main="Beta")
	lines (x=normX, y=normY, type="l", col="red")
})
rk.graph.off ()
})
