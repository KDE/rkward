local({
## Prepare
## Compute
## Print result
rk.header ("Poisson density function", list ("Lower quantile", "0", "Upper quantile", "12", "Mean", "5.00000000", "Scale", "normal", "Function", "dpois"));

rk.graph.on ()
try ({
	curve (dpois(x, lambda=5.00000000), from=0, to=12, n=13, type="p")
})
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::plot_poisson_distribution", settings="function.string=d\nlog.state=0\nmax.real=12.000000\nmean.real=5.00000000\nmin.real=0.000000\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00000000\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=p\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=", label="Run again")
.rk.make.hr()
