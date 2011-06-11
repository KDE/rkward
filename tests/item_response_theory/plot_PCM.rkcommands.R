local({
## Prepare
require(eRm)
## Compute
## Print result
rk.header("Partial credit model plot")

rk.graph.on()
try(plotICC(estimates.pcm, item.subset=c(3:6), mplot=TRUE, ask=FALSE))
rk.graph.off()
})
.rk.rerun.plugin.link(plugin="rkward::plot_pcm", settings="annotation.string=legend\nchk_ask.state=\nchk_mplot.state=mplot\ninp_items.text=3:6\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\nspin_abilfrom.real=-4.00\nspin_abilto.real=4.00\nspin_probfrom.real=0.00\nspin_probto.real=1.00\nx.available=estimates.pcm", label="Run again")
.rk.make.hr()
