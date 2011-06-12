local({
## Prepare
## Compute
source (file="source.R", local=TRUE, verbose=FALSE, print.eval=FALSE, chdir=FALSE)
## Print result
rk.header("Source R file", parameters=list("File", "source.R"))
})
local({
## Prepare
## Compute
source (file="source.R", local=FALSE, echo=TRUE, max.deparse.length=150, verbose=FALSE, print.eval=FALSE, chdir=FALSE)
## Print result
rk.header("Source R file", parameters=list("File", "source.R"))
})
