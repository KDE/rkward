local({
## Prepare
## Compute
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
.rk.rerun.plugin.link(plugin="rkward::plot_log_normal_clt", settings="drawnorm.state=1\nfunction.string=hist\nhistogram_opt.addtoplot.state=\nhistogram_opt.angle.real=45\nhistogram_opt.barlabels.state=\nhistogram_opt.density.real=7.00\nhistogram_opt.doborder.state=\nhistogram_opt.freq.state=0\nhistogram_opt.histbreaksFunction.string=Scott\nhistogram_opt.histlinetype.string=solid\nhistogram_opt.rightclosed.state=1\nhistogram_opt.usefillcol.state=\nmean.real=0.0\nnAvg.real=10.00\nnDist.real=1000.00\nnormlinecol.color.string=red\nnormpointtype.string=l\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\nscalenorm.state=0\nsd.real=1.00", label="Run again")
.rk.make.hr()
