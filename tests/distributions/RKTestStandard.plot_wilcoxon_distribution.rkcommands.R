local({
## Prepare
## Compute
## Print result
rk.header ("Binomial density function", list ("Lower quantile", "0", "Upper quantile", "24", "First sample size", "4", "Second sample size", "6", "Scale", "normal", "Function", "dwilcox"));

rk.graph.on ()
try ({
	curve (dwilcox(x, m=4, n=6), from=0, to=24, n=25, type="p")
})
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::plot_wilcoxon_distribution", settings="function.string=d\nlog.state=0\nmax.real=24.000000\nmin.real=0.000000\nnm.real=4.000000\nnn.real=6.000000\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00000000\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=p\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=", label="Run again")
.rk.make.hr()
