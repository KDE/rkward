local({
## Prepare
require(ltm)
## Compute
estimates.grm <- grm(Environment)
## Print result
rk.header ("GRM parameter estimation")
rk.print ("Call:")
rk.print.literal (deparse(estimates.grm$call, width.cutoff=500))
rk.header ("Coefficients:", level=4)
rk.print (coef(estimates.grm))
rk.print (paste("Log-likelihood value at convergence:",round(estimates.grm$log.Lik, digits=1)))
# keep results in current workspace
.GlobalEnv$estimates.grm <- estimates.grm
})
.rk.rerun.plugin.link(plugin="rkward::par_est_grm", settings="constraint.state=\ndig_abbrv.real=6.00\nghk_grm.real=21.00\nhessian.state=\nirtparam.state=TRUE\niterqn_grm.real=150.00\nnaaction.state=\noptimeth.string=BFGS\nsave_name.active=1\nsave_name.objectname=estimates.grm\nsave_name.parent=.GlobalEnv\nstartval.string=NULL\nverbose.state=\nx.available=Environment", label="Run again")
.rk.make.hr()
