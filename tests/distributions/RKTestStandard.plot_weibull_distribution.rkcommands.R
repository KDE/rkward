local({
## Prepare
## Compute
## Print result
rk.header ("Weibull density function", list ("Number of Observations", "100", "Lower quantile", "0.00", "Upper quantile", "5.00", "Shape", "2.00", "Scale", "1.00", "Scale", "normal", "Function", "dweibull"));

rk.graph.on ()
try ({
	curve (dweibull(x, shape=2.00, scale=1.00), from=0.00, to=5.00, n=100)
})
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::plot_weibull_distribution", settings="function.string=d\nlog.state=0\nmax.real=5.00\nmin.real=0.00\nn.real=100.00\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\nscale.real=1.00\nshape.real=2.00", label="Run again")
.rk.make.hr()
