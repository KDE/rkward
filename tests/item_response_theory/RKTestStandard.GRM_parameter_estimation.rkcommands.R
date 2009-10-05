local({
## Prepare
  require(ltm)
## Compute
estimates.grm <<- grm(Environment)
## Print result
rk.header ("GRM parameter estimation")
rk.print (estimates.grm)
})
.rk.rerun.plugin.link(plugin="rkward::par_est_grm", settings="constraint.state=\ndig_abbrv.real=6.00\nghk_grm.real=21.00\nhessian.state=\nirtparam.state=TRUE\niterqn_grm.real=150.00\nnaaction.state=\noptimeth.string=BFGS\nstartval.string=NULL\nverbose.state=\nx.available=Environment", label="Run again")
.rk.make.hr()
