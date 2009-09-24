local({
## Prepare
## Compute
## Print result
# parameters:
df1 <- 5.00000000; df2 <- 5.00000000; ncp <- 0.00000000;
# generate the entire data:
data <- matrix(rf(n=10000, df1=df1, df2=df2, ncp=ncp), nrow=10);
# get the sample averages:
avg <- colMeans(data);
dist.hist <- hist(avg, plot=FALSE, breaks="Sturges");
rk.graph.on ()
try ({
	plot(dist.hist, freq=FALSE, lty="solid", density=-1, xlab="Sample Averages", main="F")
})
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::plot_f_clt", settings="df1.real=5.00000000\ndf2.real=5.00000000\ndrawnorm.state=0\nfunction.string=hist\nhistogram_opt.addtoplot.state=\nhistogram_opt.barlabels.state=\nhistogram_opt.density.real=-1.000000\nhistogram_opt.doborder.state=1\nhistogram_opt.freq.state=0\nhistogram_opt.histbordercol.color.string=\nhistogram_opt.histbreaksFunction.string=Sturges\nhistogram_opt.histlinetype.string=solid\nhistogram_opt.rightclosed.state=1\nhistogram_opt.usefillcol.state=\nnAvg.real=10.000000\nnDist.real=1000.000000\nncp.real=0.00000000\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00000000\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\nscalenorm.state=0", label="Run again")
.rk.make.hr()
