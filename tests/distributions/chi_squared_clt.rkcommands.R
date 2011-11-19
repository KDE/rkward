local({
## Print result
# parameters:
df <- 4.0; ncp <- 0.6;
# mean and variances of the distribution of sample averages:
avg.exp <- df + ncp;
avg.var <- (2*df + 4*ncp)/12;
# generate the entire data:
data <- matrix(rchisq(n=12000, df=df, ncp=ncp), nrow=12);
# get the sample averages:
avg <- colMeans(data);
# normalise the variables:
avg <- (avg - avg.exp)/sqrt(avg.var);
# generate random normal samples:
normX <- seq(from=min(avg), to=max(avg), length=1000);
normY <- pnorm (normX);
rk.graph.on ()
try ({
	plot(ecdf(avg), xlab="Sample Averages", main="Chi-Squared", verticals=TRUE, do.points=FALSE)
	lines (x=normX, y=normY, type="l", col="red")
})
rk.graph.off ()
})
