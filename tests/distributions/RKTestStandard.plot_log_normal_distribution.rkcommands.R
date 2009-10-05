local({
## Prepare
## Compute
## Print result
rk.header ("Lognormal distribution function", list ("Number of Observations", "100", "Lower quantile", "0.01", "Upper quantile", "3.29", "Mean", "4.00", "Standard deviation", "1.00", "Scale", "normal", "Tail","Lower", "Function", "plnorm"));

rk.graph.on ()
try ({
	curve (plnorm(x, meanlog=4.00, sdlog=1.00, lower.tail = TRUE), from=0.01, to=3.29, n=100)
})
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::plot_lognormal_distribution", settings="function.string=p\nlog.state=0\nlower.state=1\nmax.real=3.29\nmean.real=4.00\nmin.real=0.01\nn.real=100.00\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\nsd.real=1.00", label="Run again")
.rk.make.hr()
