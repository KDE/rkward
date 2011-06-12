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
