local({
## Prepare
  require(ltm)
## Compute
estimates.grm <- grm(Environment)
## Print result
rk.header ("GRM parameter estimation")
rk.print (paste("Call: <code>",deparse(estimates.grm$call, width.cutoff=500),"</code>"))
rk.print ("<h4>Coefficients:</h4>")
rk.print (coef(estimates.grm))
rk.print (paste("Log-likelihood value at convergence:",round(estimates.grm$log.Lik, digits=1)))
# keep results in current workspace
estimates.grm <<- estimates.grm
})
.rk.rerun.plugin.link(plugin="rkward::par_est_grm", settings="chk_save.state=save\nconstraint.state=\ndig_abbrv.real=6.00\nghk_grm.real=21.00\nhessian.state=\nirtparam.state=TRUE\niterqn_grm.real=150.00\nnaaction.state=\noptimeth.string=BFGS\nsave_name.selection=estimates.grm\nstartval.string=NULL\nverbose.state=\nx.available=Environment", label="Run again")
.rk.make.hr()
