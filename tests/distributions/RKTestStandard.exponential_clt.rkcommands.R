local({
## Prepare
## Compute
## Print result
# parameters:
rate <- 1.0;
# mean and variances of the distribution of sample averages:
avg.exp <- 1/rate;
avg.var <- (1/(rate^2))/10;
# generate the entire data:
data <- matrix(rexp(n=10000, rate=rate), nrow=10);
# get the sample averages:
avg <- colMeans(data);
# normalise the variables:
avg <- (avg - avg.exp)/sqrt(avg.var);
# generate random normal samples:
normX <- seq(from=min(avg), to=max(avg), length=1000);
normY <- dnorm (normX);
dist.hist <- hist(avg, plot=FALSE, breaks="Sturges");
# calculate the ylims appropriately:
ylim <- c(0,max(c(dist.hist$density, normY)));
rk.graph.on ()
try ({
	plot(dist.hist, ylim=ylim, freq=FALSE, lty="solid", density=-1, xlab="Sample Averages", main="Exponential")
	lines (x=normX, y=normY, type="h", col="grey4")
})
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::plot_exponential_clt", settings="drawnorm.state=1\nfunction.string=hist\nhistogram_opt.addtoplot.state=\nhistogram_opt.barlabels.state=\nhistogram_opt.density.real=-1.00\nhistogram_opt.doborder.state=1\nhistogram_opt.freq.state=0\nhistogram_opt.histbordercol.color.string=\nhistogram_opt.histbreaksFunction.string=Sturges\nhistogram_opt.histlinetype.string=solid\nhistogram_opt.rightclosed.state=1\nhistogram_opt.usefillcol.state=\nnAvg.real=10.00\nnDist.real=1000.00\nnormlinecol.color.string=grey4\nnormpointtype.string=h\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\nrate.real=1.0\nscalenorm.state=1", label="Run again")
.rk.make.hr()
