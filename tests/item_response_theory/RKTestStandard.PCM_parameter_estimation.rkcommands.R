local({
## Prepare
  require(eRm)
## Compute
estimates.pcm <<- PCM(pcmdat)
## Print result
rk.header ("PCM  parameter estimation")
rk.print (estimates.pcm)
})
.rk.rerun.plugin.link(plugin="rkward::par_est_pcm", settings="design.string=auto\netastart.string=NULL\nstderr.state=se\nsumnull.state=sum0\nx.available=pcmdat", label="Run again")
.rk.make.hr()
