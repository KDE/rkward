local({
## Prepare
## Compute
## Print result
rk.header ("Chisquare density function", list ("Number of Observations", "100", "Lower quantile", "0.30000000", "Upper quantile", "24.10000000", "Degrees of freedom", "4.00000000", "Non-centrality parameter", "0.00000000", "Scale", "normal", "Function", "dchisq"));

rk.graph.on ()
try ({
	curve (dchisq(x, df=4.00000000, ncp=0.00000000), from=0.30000000, to=24.10000000, n=100)
})
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::plot_chi_squared_distribution", settings="df.real=4.00000000\nfunction.string=d\nlog.state=0\nmax.real=24.10000000\nmin.real=0.30000000\nn.real=100.000000\nncp.real=0.00000000\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00000000\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=", label="Run again")
.rk.make.hr()
