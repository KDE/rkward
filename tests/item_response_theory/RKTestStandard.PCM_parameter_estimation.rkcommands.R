local({
## Prepare
  require(eRm)
## Compute
estimates.pcm <- PCM(pcmdat)
## Print result
rk.header ("PCM  parameter estimation")
rk.print (estimates.pcm)
# keep results in current workspace
estimates.pcm <<- estimates.pcm
})
.rk.rerun.plugin.link(plugin="rkward::par_est_pcm", settings="chk_save.state=save\ndesign.string=auto\netastart.string=NULL\nsave_name.selection=estimates.pcm\nstderr.state=se\nsumnull.state=sum0\nx.available=pcmdat", label="Run again")
.rk.make.hr()
