local({
## Prepare
## Compute
## Print result
rk.header ("F density function", list ("Number of Observations", "100", "Lower quantile", "0.001", "Upper quantile", "25.00", "Numerator degrees of freedom", "5.00", "Denominator degrees of freedom", "5.00", "Non-centrality", "0", "Scale", "logarithmic", "Function", "df"));

rk.graph.on ()
try ({
	curve (df(x, df1=5.00, df2=5.00, ncp=0, log=TRUE), from=0.001, to=25.00, n=100)
})
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::plot_f_distribution", settings="df1.real=5.00\ndf2.real=5.00\nfunction.string=d\nlog.state=1\nmax.real=25.00\nmin.real=0.001\nn.real=100.00\nncp.real=0\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=", label="Run again")
.rk.make.hr()
