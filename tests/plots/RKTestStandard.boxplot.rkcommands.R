local({
## Prepare
## Compute
## Print result
	data_list <- list (women[["weight"]], women[["height"]])		#convert single sample variables to list
rk.header ("Boxplot", list ("Variable(s)", rk.get.description (women[["weight"]], women[["height"]], paste.sep=", ")))
rk.graph.on()
try (boxplot (data_list, notch = FALSE, outline = TRUE, horizontal = FALSE)) #actuall boxplot function
	try (points(apply(data.frame(women[["weight"]], women[["height"]]),2,mean,na.rm = TRUE),pch=15, cex = 1.00000000, col="blue")) #calculates the mean for all data and adds a point at the corresponding position
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::box_plot", settings="cex_sd_mean.real=1.00000000\nmean.state=TRUE\nnames_exp.text=names (x)\nnames_mode.string=default\nnotch.state=FALSE\norientation.string=FALSE\noutline.state=TRUE\npch_mean.real=15.000000\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00000000\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\nsd.state=\nsd_mean_color.color.string=blue\nx.available=women[[\\\"weight\\\"]]\\nwomen[[\\\"height\\\"]]", label="Run again")
.rk.make.hr()
