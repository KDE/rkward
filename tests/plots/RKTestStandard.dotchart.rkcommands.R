local({
## Prepare
## Compute
## Print result
x <- women[["height"]]
if (!is.numeric (x)) {
	warning ("Data may not be numeric, but proceeding as requested.\nDid you forget to check the tabulate option?")
}

rk.header ("Dot chart", parameters=list ("Variable", rk.get.description (women[["height"]]), "Tabulate", "No"))

rk.graph.on ()
try ({
names(x) <- women$weight
	dotchart(x, main="This is a test", sub="This is a subtitle")
})
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::dotchart", settings="names_exp.text=women$weight\nnames_mode.string=rexp\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00000000\nplotoptions.main.text=This is a test\nplotoptions.mainisquote.state=1\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=This is a subtitle\nplotoptions.subisquote.state=1\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\ntabulate.state=0\nx.available=women[[\\\"height\\\"]]", label="Run again")
.rk.make.hr()
local({
## Prepare
## Compute
## Print result
x <- table (warpbreaks[["tension"]], exclude=NULL)

rk.header ("Dot chart", parameters=list ("Variable", rk.get.description (warpbreaks[["tension"]]), "Tabulate", "Yes"))

rk.graph.on ()
try ({
	dotchart(x)
})
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::dotchart", settings="names_exp.text=names (x)\nnames_mode.string=default\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00000000\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\ntabulate.state=1\nx.available=warpbreaks[[\\\"tension\\\"]]", label="Run again")
.rk.make.hr()
