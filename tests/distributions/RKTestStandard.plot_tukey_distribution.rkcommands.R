local({
## Prepare
## Compute
## Print result
rk.header ("Tukey distribution function", list ("Number of Observations", "101", "Lower quantile", "-1.00", "Upper quantile", "8.00", "Sample size for range", "6", "Degreed of freedom for s", "5.00", "Number of groups", "1", "Scale", "normal", "Tail","Lower", "Function", "ptukey"));

rk.graph.on ()
try ({
	curve (ptukey(x, nmeans=6, df=5.00, nranges=1, lower.tail = TRUE), from=-1.00, to=8.00, n=101)
})
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::plot_tukey_distribution", settings="df.real=5.00\nlog.state=0\nlower.state=1\nmax.real=8.00\nmin.real=-1.00\nn.real=101.00\nnmeans.real=6.00\nnranges.real=1.00\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=", label="Run again")
.rk.make.hr()
