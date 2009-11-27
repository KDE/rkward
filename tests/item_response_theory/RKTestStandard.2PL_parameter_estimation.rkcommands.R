local({
## Prepare
  require(ltm)
## Compute
estimates.2pl <- ltm(WIRS ~ z1 * z2)
## Print result
rk.header ("2PL parameter estimation")
rk.print (estimates.2pl)
# keep results in current workspace
estimates.2pl <<- estimates.2pl
})
.rk.rerun.plugin.link(plugin="rkward::par_est_2pl", settings="chk_save.state=save\nconstraint.available=\nghk_2pl.real=15.00\ninteract.state=TRUE\nirtparam.state=TRUE\niterem.real=40.00\niterqn_2pl.real=150.00\nnaaction.state=\noptimeth.string=BFGS\nsave_name.selection=estimates.2pl\nstartval.string=NULL\nverbose.state=\nx.available=WIRS", label="Run again")
.rk.make.hr()
