local({
## Prepare
require(ltm)
## Compute
estimates.rasch <- rasch(LSAT)
## Print result
rk.header ("Rasch parameter estimation")
rk.print ("Call:")
rk.print.literal (deparse(estimates.rasch$call, width.cutoff=500))
rk.header ("Coefficients:", level=4)
rk.print (coef(estimates.rasch))
rk.print (paste("Log-likelihood value at convergence:",round(estimates.rasch$log.Lik, digits=1)))
# keep results in current workspace
.GlobalEnv$estimates.rasch <- estimates.rasch
})
