local({
## Prepare
  require(eRm)
## Compute
estimates.rsm <<- RSM(rsmdat)
## Print result
rk.header ("RSM  parameter estimation")
rk.print (estimates.rsm)
})
.rk.rerun.plugin.link(plugin="rkward::par_est_rsm", settings="design.string=auto\netastart.string=NULL\nstderr.state=se\nsumnull.state=sum0\nx.available=rsmdat", label="Run again")
.rk.make.hr()
