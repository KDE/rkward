local({
## Prepare
## Compute
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
.rk.rerun.plugin.link(plugin="rkward::plot_chi_squared_clt", settings="df.real=4.0\ndist_stepfun.addtoplot.state=\ndist_stepfun.col_hor.color.string=\ndist_stepfun.col_y0.color.string=\ndist_stepfun.col_y1.color.string=\ndist_stepfun.do_points.state=\ndist_stepfun.linetype.string=\ndist_stepfun.verticals.state=1\ndrawnorm.state=1\nfunction.string=dist\nnAvg.real=12.00\nnDist.real=1000.00\nncp.real=0.6\nnormlinecol.color.string=red\nnormpointtype.string=l\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\nscalenorm.state=1", label="Run again")
.rk.make.hr()
