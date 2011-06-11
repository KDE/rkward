local({
## Prepare
## Compute
## Print result
rk.header ("Exponential distribution function", list ("Number of Observations", "100", "Lower quantile", "0.00", "Upper quantile", "10.00", "Rate", "1.00", "Scale", "normal", "Tail","Upper", "Function", "pexp"));

rk.graph.on ()
try ({
	curve (pexp(x, rate=1.00, lower.tail = FALSE), from=0.00, to=10.00, n=100)
})
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::plot_exponential_distribution", settings="function.string=p\nlog.state=0\nlower.state=0\nmax.real=10.00\nmin.real=0.00\nn.real=100.00\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\nrate.real=1.00", label="Run again")
.rk.make.hr()
