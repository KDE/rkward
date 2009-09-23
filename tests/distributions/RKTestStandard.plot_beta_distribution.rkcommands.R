local({
## Prepare
## Compute
## Print result
rk.header ("Beta distribution function", list ("Number of Observations", "100", "Lower quantile", "0.00000000", "Upper quantile", "1.00000000", "Shape1", "2.00000000", "Shape2", "2.00000000", "Non-centrality parameter", "0.00000000", "Scale", "logarithmic", "Tail","Lower", "Function", "pbeta"));

rk.graph.on ()
try ({
	curve (pbeta(x, shape1=2.00000000, shape2=2.00000000, ncp=0.00000000, log.p=TRUE, lower.tail = TRUE), from=0.00000000, to=1.00000000, n=100)
})
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::plot_beta_distribution", settings="a.real=2.00000000\nb.real=2.00000000\nfunction.string=p\nlog.state=1\nlower.state=1\nmax.real=1.00000000\nmin.real=0.00000000\nn.real=100.000000\nncp.real=0.00000000\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00000000\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=", label="Run again")
.rk.make.hr()
