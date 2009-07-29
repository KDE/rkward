local({
## Prepare
  require(ltm)
## Compute
estimates.2pl <<- ltm(WIRS ~ z1 * z2)
## Print result
rk.header ("2PL parameter estimation")
rk.print (estimates.2pl)
})
.rk.rerun.plugin.link(plugin="rkward::par_est_2pl", settings="constraint.available=\nghk_2pl.real=15.000000\ninteract.state=TRUE\nirtparam.state=TRUE\niterem.real=40.000000\niterqn_2pl.real=150.000000\nnaaction.state=\noptimeth.string=BFGS\nstartval.string=NULL\nverbose.state=\nx.available=WIRS", label="Run again")
.rk.make.hr()
