local({
## Prepare
## Compute
## Print result
rk.header ("Geometric density function", list ("Lower quantile", "0", "Upper quantile", "12", "Probability of success on each trial", "0.50000000", "Scale", "normal", "Function", "dgeom"));

rk.graph.on ()
try ({
	curve (dgeom(x, prob=0.50000000), from=0, to=12, n=13, type="p")
})
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::plot_geometric_distribution", settings="function.string=d\nlog.state=0\nmax.real=12.000000\nmin.real=0.000000\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00000000\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=p\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\nprob.real=0.50000000", label="Run again")
.rk.make.hr()
