local({
## Prepare
  require(eRm)
## Compute
estimates.rsm <- RSM(rsmdat)
## Print result
rk.header ("RSM  parameter estimation")
rk.print (estimates.rsm)
# keep results in current workspace
estimates.rsm <<- estimates.rsm
})
.rk.rerun.plugin.link(plugin="rkward::par_est_rsm", settings="chk_save.state=save\ndesign.string=auto\netastart.string=NULL\nsave_name.selection=estimates.rsm\nstderr.state=se\nsumnull.state=sum0\nx.available=rsmdat", label="Run again")
.rk.make.hr()
