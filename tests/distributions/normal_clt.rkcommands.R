local({
## Prepare
## Compute
## Print result
# parameters:
mean <- 0.0; sd <- 1.0;
# mean and variances of the distribution of sample averages:
avg.exp <- mean;
avg.var <- (sd^2)/10;
# generate the entire data:
data <- matrix(rnorm(n=10000, mean=mean, sd=sd), nrow=10);
# get the sample averages:
avg <- colMeans(data);
# generate random normal samples:
normX <- seq(from=min(avg), to=max(avg), length=1000);
normY <- pnorm (normX, mean = avg.exp, sd = sqrt(avg.var));
rk.graph.on ()
try ({
	plot(ecdf(avg), xlab="Sample Averages", main="Normal", verticals=TRUE, col.vert="blue", do.points=FALSE, col.hor="blue", col.01line=c("gold","cyan"))
	lines (x=normX, y=normY, type="l", col="red")
})
rk.graph.off ()
})
