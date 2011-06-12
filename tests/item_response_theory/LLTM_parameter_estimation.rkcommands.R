local({
## Prepare
require(eRm)
## Compute
estimates.lltm <- LLTM(lltmdat1, mpoints=2)
## Print result
rk.header ("LLTM  parameter estimation")
rk.print ("Call:")
rk.print.literal (deparse(estimates.lltm$call, width.cutoff=500))
rk.header ("Coefficients:", level=4)
rk.print(t(rbind(Eta=estimates.lltm$etapar,StdErr=estimates.lltm$se.eta)))
rk.print (paste("Conditional log-likelihood:",round(estimates.lltm$loglik, digits=1),
"<br />Number of iterations:",estimates.lltm$iter,"<br />Number of parameters:",estimates.lltm$npar))
# keep results in current workspace
.GlobalEnv$estimates.lltm <- estimates.lltm
})
