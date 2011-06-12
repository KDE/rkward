local({
## Prepare
require(eRm)
## Compute
estimates.pcm <- PCM(pcmdat)
## Print result
rk.header ("PCM  parameter estimation")
rk.print ("Call:")
rk.print.literal (deparse(estimates.pcm$call, width.cutoff=500))
rk.header ("Coefficients:", level=4)
rk.print(t(rbind(Eta=estimates.pcm$etapar,StdErr=estimates.pcm$se.eta)))
rk.print (paste("Conditional log-likelihood:",round(estimates.pcm$loglik, digits=1),
"<br />Number of iterations:",estimates.pcm$iter,"<br />Number of parameters:",estimates.pcm$npar))
# keep results in current workspace
.GlobalEnv$estimates.pcm <- estimates.pcm
})
