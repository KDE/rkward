local({
## Prepare
## Compute
## Print result
rk.header ("Uniform density function", list ("Number of Observations", "100", "Lower quantile", "-1.00000000", "Upper quantile", "2.00000000", "Minimum", "0.00000000", "Maximum", "1.00000000", "Scale", "normal", "Function", "dunif"));

rk.graph.on ()
try ({
	curve (dunif(x, min=0.00000000, max=1.00000000), from=-1.00000000, to=2.00000000, n=100)
})
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::plot_uniform_distribution", settings="function.string=d\nllim.real=0.00000000\nlog.state=0\nmax.real=2.00000000\nmin.real=-1.00000000\nn.real=100.000000\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00000000\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\nulim.real=1.00000000", label="Run again")
.rk.make.hr()
