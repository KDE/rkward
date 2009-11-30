local({
## Prepare
  require(eRm)
## Compute
estimates.pcm <- PCM(pcmdat)
## Print result
rk.header ("PCM  parameter estimation")
rk.print (paste("Call: <code>",deparse(estimates.pcm$call, width.cutoff=500),"</code>"))
rk.print ("<h4>Coefficients:</h4>")
rk.print(t(rbind(Eta=estimates.pcm$etapar,StdErr=estimates.pcm$se.eta)))
rk.print (paste("Conditional log-likelihood:",round(estimates.pcm$loglik, digits=1),
"<br />Number of iterations:",estimates.pcm$iter,"<br />Number of parameters:",estimates.pcm$npar))
# keep results in current workspace
estimates.pcm <<- estimates.pcm
})
.rk.rerun.plugin.link(plugin="rkward::par_est_pcm", settings="chk_save.state=save\ndesign.string=auto\netastart.string=NULL\nsave_name.selection=estimates.pcm\nstderr.state=se\nsumnull.state=sum0\nx.available=pcmdat", label="Run again")
.rk.make.hr()
