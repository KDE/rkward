local({
## Prepare
  require(ltm)
## Compute
## Print result
rk.header("Graded response model plot")

rk.graph.on()
try(plot(estimates.grm, type="ICC", items=c(6)))
rk.graph.off()
})
.rk.rerun.plugin.link(plugin="rkward::plot_grm", settings="annotation.string=annot\ninp_items.text=6\nplot_type.string=items\nplot_type_item.string=ICC\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\nspin_categ.real=0.00\nspin_from.real=-3.80\nspin_to.real=3.80\nx.available=estimates.grm", label="Run again")
.rk.make.hr()
