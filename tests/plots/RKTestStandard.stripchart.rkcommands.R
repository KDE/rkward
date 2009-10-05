local({
## Prepare
## Compute
## Print result
rk.header ("Stripchart", list ("Variable"=rk.get.description (warpbreaks[["breaks"]]), "Group"=rk.get.description (warpbreaks[["tension"]]), "Method"="stack", "Offset" = 0.50, "Orientation"="Horizontal"))

rk.graph.on ()
try (stripchart (warpbreaks[["breaks"]] ~ (warpbreaks[["tension"]]), method = "stack", offset = 0.50))
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::stripchart", settings="g.available=warpbreaks[[\\\"tension\\\"]]\nmethod.string=stack\noffset.real=0.50\norientation.string=Horizontal\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\nx.available=warpbreaks[[\\\"breaks\\\"]]", label="Run again")
.rk.make.hr()
