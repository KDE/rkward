local({
## Prepare
## Compute
## Print result
rk.header ("Cauchy density function", list ("Number of Observations", "100", "Lower quantile", "-3.29000000", "Upper quantile", "3.29000000", "Location", "0.00000000", "Scale", "1.00000000", "Scale", "normal", "Function", "dcauchy"));

rk.graph.on ()
try ({
	curve (dcauchy(x, location=0.00000000, scale=1.00000000), from=-3.29000000, to=3.29000000, n=100)
})
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::plot_cauchy_distribution", settings="function.string=d\nloc.real=0.00000000\nlog.state=0\nmax.real=3.29000000\nmin.real=-3.29000000\nn.real=100.000000\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00000000\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\nscale.real=1.00000000", label="Run again")
.rk.make.hr()
