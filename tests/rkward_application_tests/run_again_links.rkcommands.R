local({
## Prepare
## Compute
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
.rk.get.structure.global (".Random.seed")
.rk.rerun.plugin.link(plugin="rkward::plot_beta_clt", settings="a.real=2.0\nb.real=2.0\ndrawnorm.state=1\nfunction.string=hist\nhistogram_opt.barlabels.state=1\nhistogram_opt.density.real=-1.00\nhistogram_opt.doborder.state=1\nhistogram_opt.histbordercol.color.string=\nhistogram_opt.histbreaksFunction.string=Sturges\nhistogram_opt.histfillcol.color.string=azure\nhistogram_opt.histlinetype.string=solid\nhistogram_opt.rightclosed.state=1\nhistogram_opt.usefillcol.state=1\nnAvg.real=10.00\nnDist.real=1000.00\nnormlinecol.color.string=red\nnormpointtype.string=l\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nscalenorm.state=0", label="Run again")
.rk.make.hr()
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
.rk.get.structure.global (".Random.seed")
.rk.rerun.plugin.link(plugin="rkward::plot_normal_clt", settings="dist_stepfun.col_hor.color.string=blue\ndist_stepfun.col_y0.color.string=gold\ndist_stepfun.col_y1.color.string=cyan\ndist_stepfun.do_points.state=\ndist_stepfun.linetype.string=\ndist_stepfun.verticals.state=1\ndrawnorm.state=1\nfunction.string=dist\nmean.real=0.0\nnAvg.real=10.00\nnDist.real=1000.00\nnormlinecol.color.string=red\nnormpointtype.string=l\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nscalenorm.state=0\nsd.real=1.0", label="Run again")
.rk.make.hr()
