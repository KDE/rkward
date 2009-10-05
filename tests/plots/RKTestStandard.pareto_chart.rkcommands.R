local({
## Prepare
require(qcc)
require(xtable)
## Compute
## Print result
x <- swiss[["Catholic"]]
if (!is.numeric (x)) {
	warning ("Data may not be numeric, but proceeding as requested.\nDid you forget to check the tabulate option?")
}

rk.header ("Pareto chart")

rk.graph.on ()
try ({
	descriptives <- pareto.chart(x, main="swiss[[\"Catholic\"]]")
	rk.results(xtable(descriptives))
})
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::pareto", settings="descriptives.state=TRUE\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\ntabulate.state=FALSE\nx.available=swiss[[\\\"Catholic\\\"]]", label="Run again")
.rk.make.hr()
