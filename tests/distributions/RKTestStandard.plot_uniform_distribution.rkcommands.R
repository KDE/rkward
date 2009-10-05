local({
## Prepare
## Compute
## Print result
rk.header ("Uniform density function", list ("Number of Observations", "100", "Lower quantile", "-1.00", "Upper quantile", "2.00", "Minimum", "0.00", "Maximum", "1.00", "Scale", "normal", "Function", "dunif"));

rk.graph.on ()
try ({
	curve (dunif(x, min=0.00, max=1.00), from=-1.00, to=2.00, n=100)
})
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::plot_uniform_distribution", settings="function.string=d\nllim.real=0.00\nlog.state=0\nmax.real=2.00\nmin.real=-1.00\nn.real=100.00\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\nulim.real=1.00", label="Run again")
.rk.make.hr()
