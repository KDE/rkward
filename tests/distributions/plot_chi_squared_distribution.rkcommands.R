local({
## Prepare
## Compute
## Print result
rk.header ("Chisquare density function", list ("Number of Observations", "100", "Lower quantile", "0.30", "Upper quantile", "24.10", "Degrees of freedom", "4.00", "Non-centrality parameter", "0.00", "Scale", "normal", "Function", "dchisq"));

rk.graph.on ()
try ({
	curve (dchisq(x, df=4.00, ncp=0.00), from=0.30, to=24.10, n=100)
})
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::plot_chi_squared_distribution", settings="df.real=4.00\nfunction.string=d\nlog.state=0\nmax.real=24.10\nmin.real=0.30\nn.real=100.00\nncp.real=0.00\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=", label="Run again")
.rk.make.hr()
