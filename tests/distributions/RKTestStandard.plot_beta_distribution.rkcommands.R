local({
## Prepare
## Compute
## Print result
rk.header ("Beta distribution function", list ("Number of Observations", "100", "Lower quantile", "0.00", "Upper quantile", "1.00", "Shape1", "2.00", "Shape2", "2.00", "Non-centrality parameter", "0.00", "Scale", "logarithmic", "Tail","Lower", "Function", "pbeta"));

rk.graph.on ()
try ({
	curve (pbeta(x, shape1=2.00, shape2=2.00, ncp=0.00, log.p=TRUE, lower.tail = TRUE), from=0.00, to=1.00, n=100)
})
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::plot_beta_distribution", settings="a.real=2.00\nb.real=2.00\nfunction.string=p\nlog.state=1\nlower.state=1\nmax.real=1.00\nmin.real=0.00\nn.real=100.00\nncp.real=0.00\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=", label="Run again")
.rk.make.hr()
