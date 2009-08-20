local({
## Prepare
## Compute
package.skeleton(name="anRpackage", list=c('rktest.setSuiteStandards','rktest.runRKTestSuite'), path=".", force= TRUE)
## Print result
rk.header("Create package skeleton", parameters=list("Name", "anRpackage",
	"Directory", "."))
})
.rk.rerun.plugin.link(plugin="rkward::save_skeleton", settings="data.available=rktest.setSuiteStandards\\nrktest.runRKTestSuite\nforce.state=TRUE\nname.text=anRpackage\npath.selection=.", label="Run again")
.rk.make.hr()
