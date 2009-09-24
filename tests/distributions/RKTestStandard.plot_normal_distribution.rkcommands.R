local({
## Prepare
## Compute
## Print result
rk.header ("Normal density function", list ("Number of Observations", "100", "Lower quantile", "-3.29000000", "Upper quantile", "3.29000000", "Mean", "0.00000000", "Standard Deviation", "1.00000000", "Scale", "normal", "Function", "dnorm"));

rk.graph.on ()
try ({
	curve (dnorm(x, mean=0.00000000, sd=1.00000000), from=-3.29000000, to=3.29000000, n=100)
})
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::plot_normal_distribution", settings="function.string=d\nlog.state=0\nmax.real=3.29000000\nmean.real=0.00000000\nmin.real=-3.29000000\nn.real=100.000000\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00000000\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\nsd.real=1.00000000", label="Run again")
.rk.make.hr()
