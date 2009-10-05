local({
## Prepare
## Compute
## Print result
rk.header ("Negative Binomial density function", list ("Lower quantile", "0", "Upper quantile", "24", "Target for number of successful trials", "12", "Probability of success in each trial", "0.75", "Scale", "normal", "Function", "dnbinom"));

rk.graph.on ()
try ({
	curve (dnbinom(x, size=12, prob=0.75), from=0, to=24, n=25, type="p")
})
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::plot_negbinomial_distribution", settings="function.string=d\nlog.state=0\nmax.real=24.00\nmin.real=0.00\nparam.string=pprob\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=p\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\nprob.real=0.75\nsize_disp.real=12.00\nsize_trial.real=12.00", label="Run again")
.rk.make.hr()
