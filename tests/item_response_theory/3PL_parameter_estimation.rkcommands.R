local({
## Prepare
require(ltm)
## Compute
estimates.3pl <- tpm(LSAT)
## Print result
rk.header ("3PL parameter estimation")
rk.print ("Call:")
rk.print.literal (deparse(estimates.3pl$call, width.cutoff=500))
rk.header ("Coefficients:", level=4)
rk.print (coef(estimates.3pl))
rk.print (paste("Log-likelihood value at convergence:",round(estimates.3pl$log.Lik, digits=1)))
# keep results in current workspace
.GlobalEnv$estimates.3pl <- estimates.3pl
})
