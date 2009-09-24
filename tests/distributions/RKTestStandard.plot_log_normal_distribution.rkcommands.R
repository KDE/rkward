local({
## Prepare
## Compute
## Print result
rk.header ("Lognormal distribution function", list ("Number of Observations", "100", "Lower quantile", "0.01000000", "Upper quantile", "3.29000000", "Mean", "4.00000000", "Standard deviation", "1.00000000", "Scale", "normal", "Tail","Lower", "Function", "plnorm"));

rk.graph.on ()
try ({
	curve (plnorm(x, meanlog=4.00000000, sdlog=1.00000000, lower.tail = TRUE), from=0.01000000, to=3.29000000, n=100)
})
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::plot_lognormal_distribution", settings="function.string=p\nlog.state=0\nlower.state=1\nmax.real=3.29000000\nmean.real=4.00000000\nmin.real=0.01000000\nn.real=100.000000\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00000000\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\nsd.real=1.00000000", label="Run again")
.rk.make.hr()
