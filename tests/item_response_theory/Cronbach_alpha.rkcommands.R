local({
## Prepare
require(ltm)
## Compute
cronalpha.res <- cronbach.alpha(subset(LSAT, select=c("Item 1", "Item 2", "Item 3", "Item 4", "Item 5")), CI=TRUE)
descript.res <- descript(subset(LSAT, select=c("Item 1", "Item 2", "Item 3", "Item 4", "Item 5")), chi.squared=FALSE, B=1000)
## Print result
rk.header ("Cronbach's alpha")
rk.print("for the 'LSAT' data-set (subset: Item 1, Item 2, Item 3, Item 4, Item 5)")
rk.print(paste("Items:",cronalpha.res$p,"<br />Sample units:",cronalpha.res$n,"<br /><strong>alpha:",round(cronalpha.res$alpha, digits=2),"</strong>"))
rk.print("Effects on alpha if items are removed:")
rk.print(descript.res$alpha)
rk.print("95% Confidence interval:")
rk.print(cronalpha.res$ci)
})
