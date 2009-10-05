local({
## Prepare
## Compute
## Print result
rk.header ("Gamma density function", list ("Number of Observations", "100", "Lower quantile", "0.01", "Upper quantile", "4.60", "Shape", "1.61", "Rate", "0.87", "Scale", "normal", "Function", "dgamma"));

rk.graph.on ()
try ({
	curve (dgamma(x, shape=1.61, rate=0.87), from=0.01, to=4.60, n=100)
})
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::plot_gamma_distribution", settings="function.string=d\nlog.state=0\nmax.real=4.60\nmin.real=0.01\nn.real=100.00\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\nrate.real=0.87\nshape.real=1.61", label="Run again")
.rk.make.hr()
