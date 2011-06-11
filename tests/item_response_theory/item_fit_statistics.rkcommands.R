local({
## Prepare
require(ltm)
## Compute
itemfit.res <- item.fit(estimates.rasch)
## Print result
rk.header ("Item-fit statistics (estimates.rasch)")
rk.print ("Call:")
rk.print.literal (deparse(itemfit.res$call, width.cutoff=500))
rk.print ("Alternative: Items do not fit the model")
rk.print (paste("Ability Categories:", itemfit.res$G))
rk.header ("Item-Fit Statistics and P-values:", level=4)
rk.print(cbind("X^2"=round(itemfit.res$Tobs, digits=3), "Pr (&gt;X^2)"=format(round(itemfit.res$p.values, digits=3), nsmall=3)))
})
.rk.rerun.plugin.link(plugin="rkward::ltm_item_fit", settings="drop_sumgroups.string=median\nrad_pvalue.string=chi2\nspin_groups.real=10.00\nx.available=estimates.rasch", label="Run again")
.rk.make.hr()
