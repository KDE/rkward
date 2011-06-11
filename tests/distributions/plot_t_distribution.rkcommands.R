local({
## Prepare
## Compute
## Print result
rk.header ("Student t distribution function", list ("Number of Observations", "100", "Minimum", "-12.924", "Maximum", "12.924", "Degrees of freedom", "1.00", "Non-centrality", "0.00", "Scale", "normal", "Tail","Lower", "Function", "pt"));

rk.graph.on ()
try ({
	curve (pt(x, df=1.00, ncp=0.00, lower.tail = TRUE), from=-12.924, to=12.924, n=100)
})
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::plot_t_distribution", settings="df.real=1.00\nfunction.string=p\nlog.state=0\nlower.state=1\nmax.real=12.924\nmin.real=-12.924\nn.real=100.00\nncp.real=0.00\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=", label="Run again")
.rk.make.hr()
