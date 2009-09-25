local({
## Prepare
require(hdrcde)
## Compute
## Print result
rk.header ("Highest density regions", list ("Variable", rk.get.description (women[["height"]]), "Band Width", "nrd0", "Adjust", 1.00000000, "Remove Missing Values", na.rm=TRUE, "Length", length (women[["height"]]), "Resolution", 512.00000000, "Smoothing Kernel", "gaussian"))

rk.graph.on ()
try ({
	hdr.den(den=density(women[["height"]], bw="nrd0", adjust=1.00000000, kern="gaussian", n=512.00000000, na.rm=TRUE))
})
rk.graph.off ()
})
.rk.rerun.plugin.link(plugin="rkward::density_plot", settings="adjust.real=1.00000000\nbw.string=nrd0\nkern.string=gaussian\nn.real=512.00000000\nnarm.state=na.rm=TRUE\nplot_type.string=hdr_plot\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00000000\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\nrug.state=0\nx.available=women[[\\\"height\\\"]]", label="Run again")
.rk.make.hr()
