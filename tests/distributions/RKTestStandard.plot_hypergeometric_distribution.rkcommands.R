local({
## Prepare
## Compute
## Print result
rk.header ("Hypergeometric distribution function", list ("Lower quantile", "0", "Upper quantile", "12", "Number of white balls", "12", "Number of black balls", "12", "Number of balls drawn", "15", "Scale", "normal", "Tail","Lower", "Function", "phyper"));

rk.graph.on ()
try ({
	curve (phyper(x, m=12, n=12, k=15, lower.tail = TRUE), from=0, to=12, n=13, type="p")
})
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::plot_hypergeometric_distribution", settings="function.string=p\nk.real=15.000000\nlog.state=0\nlower.state=1\nm.real=12.000000\nmax.real=12.000000\nmin.real=0.000000\nn_val.real=12.000000\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00000000\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=p\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=", label="Run again")
.rk.make.hr()
