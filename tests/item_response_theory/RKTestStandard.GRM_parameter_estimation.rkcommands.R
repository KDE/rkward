local({
## Prepare
  require(ltm)
## Compute
estimates.grm <<- grm(Environment)
## Print result
rk.header ("GRM parameter estimation")
rk.print (estimates.grm)
})
.rk.rerun.plugin.link(plugin="rkward::par_est_grm", settings="constraint.state=\ndig_abbrv.real=6.000000\nghk_grm.real=21.000000\nhessian.state=\nirtparam.state=TRUE\niterqn_grm.real=150.000000\nnaaction.state=\noptimeth.string=BFGS\nstartval.string=NULL\nverbose.state=\nx.available=Environment", label="Run again")
.rk.make.hr()
