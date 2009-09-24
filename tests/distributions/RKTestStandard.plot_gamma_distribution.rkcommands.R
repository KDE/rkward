local({
## Prepare
## Compute
## Print result
rk.header ("Gamma density function", list ("Number of Observations", "100", "Lower quantile", "0.01000000", "Upper quantile", "4.60000000", "Shape", "1.61000000", "Rate", "0.87000000", "Scale", "normal", "Function", "dgamma"));

rk.graph.on ()
try ({
	curve (dgamma(x, shape=1.61000000, rate=0.87000000), from=0.01000000, to=4.60000000, n=100)
})
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::plot_gamma_distribution", settings="function.string=d\nlog.state=0\nmax.real=4.60000000\nmin.real=0.01000000\nn.real=100.000000\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00000000\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\nrate.real=0.87000000\nshape.real=1.61000000", label="Run again")
.rk.make.hr()
