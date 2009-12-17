local({
## Prepare
require(ltm)
## Compute
estimates.gpcm <- gpcm(subset(Science, select=c("Work", "Industry", "Future", "Benefit")), constraint="rasch")
## Print result
rk.header ("GPCM parameter estimation")
rk.print ("Call:")
rk.print.literal (deparse(estimates.gpcm$call, width.cutoff=500))
rk.header ("Coefficients:", level=4)
rk.print (coef(estimates.gpcm))
rk.print (paste("Log-likelihood value at convergence:",round(estimates.gpcm$log.Lik, digits=1)))
# keep results in current workspace
estimates.gpcm <<- estimates.gpcm
})
.rk.rerun.plugin.link(plugin="rkward::par_est_gpcm", settings="chk_save.state=save\nchk_select.state=select\nconstraint.string=rasch\nepshess.real=0.000001\nghk_gpcm.real=21.00\ninp_items.available=Science[[\\\"Work\\\"]]\\nScience[[\\\"Industry\\\"]]\\nScience[[\\\"Future\\\"]]\\nScience[[\\\"Benefit\\\"]]\nirtparam.state=TRUE\niterqn_gpcm.real=150.00\nnaaction.state=\nnumrderiv.string=fd\noptimeth.string=BFGS\noptimizer.string=optim\nsave_name.selection=estimates.gpcm\nstartval.string=NULL\nverbose.state=\nx.available=Science", label="Run again")
.rk.make.hr()
