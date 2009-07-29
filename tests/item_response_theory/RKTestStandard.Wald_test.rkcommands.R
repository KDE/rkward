local({
## Prepare
  require(eRm)
## Compute
waldtest.res <- Waldtest(estimates.rsm)
## Print result
rk.header ("Wald test (estimates.rsm)")
rk.print (waldtest.res)
})
.rk.rerun.plugin.link(plugin="rkward::eRm_waldtest", settings="rad_splitcr.string=median\nx.available=estimates.rsm", label="Run again")
.rk.make.hr()
