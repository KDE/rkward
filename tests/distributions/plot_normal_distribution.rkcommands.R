local({
## Prepare
## Compute
## Print result
rk.header ("Normal density function", list ("Number of Observations", "100", "Lower quantile", "-3.29", "Upper quantile", "3.29", "Mean", "0.00", "Standard Deviation", "1.00", "Scale", "normal", "Function", "dnorm"));

rk.graph.on ()
try ({
	curve (dnorm(x, mean=0.00, sd=1.00), from=-3.29, to=3.29, n=100)
})
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::plot_normal_distribution", settings="function.string=d\nlog.state=0\nmax.real=3.29\nmean.real=0.00\nmin.real=-3.29\nn.real=100.00\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\nsd.real=1.00", label="Run again")
.rk.make.hr()
