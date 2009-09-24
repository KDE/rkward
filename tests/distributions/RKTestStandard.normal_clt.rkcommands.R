local({
## Prepare
## Compute
## Print result
# parameters:
mean <- 0.00000000; sd <- 1.00000000;
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
.rk.rerun.plugin.link(plugin="rkward::plot_normal_clt", settings="dist_stepfun.addtoplot.state=\ndist_stepfun.col_hor.color.string=blue\ndist_stepfun.col_y0.color.string=gold\ndist_stepfun.col_y1.color.string=cyan\ndist_stepfun.do_points.state=\ndist_stepfun.linetype.string=\ndist_stepfun.verticals.state=1\ndrawnorm.state=1\nfunction.string=dist\nmean.real=0.00000000\nnAvg.real=10.000000\nnDist.real=1000.000000\nnormlinecol.color.string=red\nnormpointtype.string=l\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00000000\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\nscalenorm.state=0\nsd.real=1.00000000", label="Run again")
.rk.make.hr()
