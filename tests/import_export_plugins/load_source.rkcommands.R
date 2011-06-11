local({
## Prepare
## Compute
source (file="source.R", local=TRUE, verbose=FALSE, print.eval=FALSE, chdir=FALSE)
## Print result
rk.header("Source R file", parameters=list("File", "source.R"))
})
.rk.rerun.plugin.link(plugin="rkward::load_source", settings="chdir.state=FALSE\necho.state=0\nfile.selection=source.R\nlocal.state=TRUE\nprinteval.state=FALSE", label="Run again")
.rk.make.hr()
local({
## Prepare
## Compute
source (file="source.R", local=FALSE, echo=TRUE, max.deparse.length=150, verbose=FALSE, print.eval=FALSE, chdir=FALSE)
## Print result
rk.header("Source R file", parameters=list("File", "source.R"))
})
.rk.rerun.plugin.link(plugin="rkward::load_source", settings="chdir.state=FALSE\necho.state=1\nfile.selection=source.R\nlocal.state=FALSE\nmaxdeparselength.real=150.00\nprinteval.state=FALSE\npromptecho.text=\nverbose.state=FALSE", label="Run again")
.rk.make.hr()
