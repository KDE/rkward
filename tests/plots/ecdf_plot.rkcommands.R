local({
## Prepare
yrange <- range (swiss[["Catholic"]], na.rm=TRUE)
data.mean <- mean (swiss[["Catholic"]], na.rm=TRUE)
data.sd <- sd (swiss[["Catholic"]], na.rm=TRUE)
## Compute
## Print result
rk.header ("Empirical Cumulative Distribution Function", list ("Variable", rk.get.description (swiss[["Catholic"]]), "Minimum", yrange[1], "Maximum", yrange[2]))

rk.graph.on ()
try ({
	plot.ecdf (swiss[["Catholic"]], , verticals=FALSE)
	curve (pnorm (x, mean=data.mean, sd=data.sd), from=yrange[1], to=yrange[2], add=TRUE, , col="blue")
	rug (swiss[["Catholic"]], 0.03, 0.50, side = 3)
})
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::ecdf_plot", settings="adjust_th_pnorm.state=1\ncol_rug.color.string=\ncol_thnorm.color.string=blue\nlwd.real=0.50\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\nrug.state=1\nside.string=side = 3\nstepfun_options.addtoplot.state=\nstepfun_options.col_hor.color.string=\nstepfun_options.col_points.color.string=\nstepfun_options.col_y0.color.string=\nstepfun_options.col_y1.color.string=\nstepfun_options.do_points.state=1\nstepfun_options.linetype.string=\nstepfun_options.verticals.state=\nth_pnorm.state=1\nticksize.real=0.03\nx.available=swiss[[\\\"Catholic\\\"]]", label="Run again")
.rk.make.hr()
