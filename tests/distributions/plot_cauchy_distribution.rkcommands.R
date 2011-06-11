local({
## Prepare
## Compute
## Print result
rk.header ("Cauchy density function", list ("Number of Observations", "100", "Lower quantile", "-3.29", "Upper quantile", "3.29", "Location", "0.00", "Scale", "1.00", "Scale", "normal", "Function", "dcauchy"));

rk.graph.on ()
try ({
	curve (dcauchy(x, location=0.00, scale=1.00), from=-3.29, to=3.29, n=100)
})
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::plot_cauchy_distribution", settings="function.string=d\nloc.real=0.00\nlog.state=0\nmax.real=3.29\nmin.real=-3.29\nn.real=100.00\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\nscale.real=1.00", label="Run again")
.rk.make.hr()
