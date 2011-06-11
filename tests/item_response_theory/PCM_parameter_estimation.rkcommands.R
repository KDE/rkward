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
.rk.rerun.plugin.link(plugin="rkward::par_est_pcm", settings="design.string=auto\netastart.string=NULL\nsave_name.active=1\nsave_name.objectname=estimates.pcm\nsave_name.parent=.GlobalEnv\nstderr.state=se\nsumnull.state=sum0\nx.available=pcmdat", label="Run again")
.rk.make.hr()
