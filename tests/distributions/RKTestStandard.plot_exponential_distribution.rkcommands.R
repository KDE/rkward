local({
## Prepare
## Compute
## Print result
rk.header ("Exponential distribution function", list ("Number of Observations", "100", "Lower quantile", "0.00000000", "Upper quantile", "10.00000000", "Rate", "1.00000000", "Scale", "normal", "Tail","Upper", "Function", "pexp"));

rk.graph.on ()
try ({
	curve (pexp(x, rate=1.00000000, lower.tail = FALSE), from=0.00000000, to=10.00000000, n=100)
})
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::plot_exponential_distribution", settings="function.string=p\nlog.state=0\nlower.state=0\nmax.real=10.00000000\nmin.real=0.00000000\nn.real=100.000000\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00000000\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\nrate.real=1.00000000", label="Run again")
.rk.make.hr()
