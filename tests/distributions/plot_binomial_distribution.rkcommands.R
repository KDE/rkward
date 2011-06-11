local({
## Prepare
## Compute
## Print result
rk.header ("Binomial density function", list ("Lower quantile", "0", "Upper quantile", "12", "Number of trials", "12", "Probability of success on each trial", "0.50", "Scale", "normal", "Function", "dbinom"));

rk.graph.on ()
try ({
	curve (dbinom(x, size=12, prob=0.50), from=0, to=12, n=13, type="p")
})
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::plot_binomial_distribution", settings="function.string=d\nlog.state=0\nmax.real=12.00\nmin.real=0.00\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=p\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\nprob.real=0.50\nsize.real=12.00", label="Run again")
.rk.make.hr()
