local({
## Compute
source (file="PATH/source.R", local=TRUE, verbose=FALSE, print.eval=FALSE, chdir=FALSE)
## Print result
rk.header ("Source R file", parameters=list("File name"="PATH/source.R"))
})
local({
## Compute
source (file="PATH/source.R", local=FALSE, echo=TRUE, max.deparse.length=150, verbose=FALSE, print.eval=FALSE, chdir=FALSE)
## Print result
rk.header ("Source R file", parameters=list("File name"="PATH/source.R"))
})
