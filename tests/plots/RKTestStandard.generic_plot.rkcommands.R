local({
## Prepare
## Compute
## Print result
rk.header ("Generic Plot")
rk.graph.on ()
try({
	plot(swiss);
})
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::plot", settings="plotoptions.add_grid.state=0\nplotoptions.asp.real=0.00000000\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\nxvarslot.available=swiss\nyvarslot.available=", label="Run again")
.rk.make.hr()
