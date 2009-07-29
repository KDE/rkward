local({
## Prepare
  require(eRm)
## Compute
## Print result
rk.header("Andersen's LR test")

rk.graph.on()
lr.res <- LRtest(estimates.pcm, se=TRUE)
try(plotGOF(lr.res, conf=list(), ctrline=list()))
rk.graph.off()
})
.rk.rerun.plugin.link(plugin="rkward::eRm_plotLR", settings="annotation.string=items\nchk_confint.state=conf\nchk_ctrline.state=ctrline\nchk_se.state=se\ninp_items.text=\nplotoptions.add_grid.state=0\nplotoptions.asp.real=0.00000000\nplotoptions.main.text=\nplotoptions.pointcolor.color.string=\nplotoptions.pointtype.string=\nplotoptions.sub.text=\nplotoptions.xaxt.state=\nplotoptions.xlab.text=\nplotoptions.xlog.state=\nplotoptions.xmaxvalue.text=\nplotoptions.xminvalue.text=\nplotoptions.yaxt.state=\nplotoptions.ylab.text=\nplotoptions.ylog.state=\nplotoptions.ymaxvalue.text=\nplotoptions.yminvalue.text=\nrad_splitcr.string=median\nspin_abilfrom.real=-3.00000000\nspin_abilto.real=3.00000000\nspin_confint.real=0.95000000\nspin_ctrline.real=0.95000000\nx.available=estimates.pcm", label="Run again")
.rk.make.hr()
