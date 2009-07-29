local({
## Prepare
  require(ltm)
## Compute
## Print result
rk.header("Birnbaum three parameter model plot")

rk.graph.on()
try(plot(estimates.3pl, type="IIC", items=0))
rk.graph.off()
})
.rk.rerun.plugin.link(plugin="rkward::plot_tpm", settings="annotation.string=annot\nplot_type.string=TIC\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00000000\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\nspin_from.real=-3.80000000\nspin_to.real=3.80000000\nx.available=estimates.3pl", label="Run again")
.rk.make.hr()
