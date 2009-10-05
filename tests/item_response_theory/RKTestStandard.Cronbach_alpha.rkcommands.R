local({
## Prepare
  require(ltm)
## Compute
cronalpha.res <- cronbach.alpha(LSAT, CI=TRUE, B=500)
## Print result
rk.header ("Cronbach's alpha (LSAT)")
rk.print (cronalpha.res)
})
.rk.rerun.plugin.link(plugin="rkward::ltm_cronbach_alpha", settings="chk_bsci.state=bsci\nchk_na.state=\nchk_standard.state=\nspin_ci.real=0.95\nspin_samples.real=500.00\nx.available=LSAT", label="Run again")
.rk.make.hr()
