local({
## Prepare
## Compute
## Print result
groups <- rk.list (warpbreaks[["tension"]], warpbreaks[["wool"]])
data_list <- split (warpbreaks[["breaks"]], groups)		#split sample by grouping variables
rk.header ("Boxplot", list ("Outcome variable", rk.get.description (warpbreaks[["breaks"]]), "Grouping variable(s)", paste (names (groups), collapse=", ")))
rk.graph.on()
try (boxplot (data_list, notch = FALSE, outline = FALSE, horizontal = TRUE)) #actuall boxplot function
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::box_plot", settings="data_mode.string=grouped_outcome\ngroups.available=warpbreaks[[\\\"tension\\\"]]\\nwarpbreaks[[\\\"wool\\\"]]\nmean.state=\nnames_exp.text=names (x)\nnames_mode.string=default\nnotch.state=FALSE\norientation.string=TRUE\noutcome.available=warpbreaks[[\\\"breaks\\\"]]\noutline.state=FALSE\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\nsd.state=\nx.available=datasets::women[[\\\"weight\\\"]]\\ndatasets::women[[\\\"height\\\"]]", label="Run again")
.rk.make.hr()
