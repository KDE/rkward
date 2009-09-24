local({
## Prepare
## Compute
## Print result
# parameters:
size <- 12; prob <- 0.28000000;
# mean and variances of the distribution of sample averages:
avg.exp <- size*(1-prob)/prob;
avg.var <- (size*(1-prob)/prob^2)/10;
# generate the entire data:
data <- matrix(rnbinom(n=10000, size=size, prob=prob), nrow=10);
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
	plot(dist.hist, ylim=ylim, freq=FALSE, lty="solid", density=-1, xlab="Sample Averages", main="Negative Binomial")
	lines (x=normX, y=normY, type="l", col="red")
})
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::plot_negbinomial_clt", settings="drawnorm.state=1\nfunction.string=hist\nhistogram_opt.addtoplot.state=\nhistogram_opt.barlabels.state=\nhistogram_opt.density.real=-1.000000\nhistogram_opt.doborder.state=1\nhistogram_opt.freq.state=0\nhistogram_opt.histbordercol.color.string=\nhistogram_opt.histbreaksFunction.string=Sturges\nhistogram_opt.histlinetype.string=solid\nhistogram_opt.rightclosed.state=1\nhistogram_opt.usefillcol.state=\nnAvg.real=10.000000\nnDist.real=1000.000000\nnormlinecol.color.string=red\nnormpointtype.string=l\nparam.string=pprob\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00000000\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\nprob.real=0.28000000\nscalenorm.state=0\nsize_disp.real=12.00000000\nsize_trial.real=12.000000", label="Run again")
.rk.make.hr()
