local({
## Prepare
## Compute
package.skeleton(name="anRpackage", list=c('skel.func1','skel.func2'), path=".", force= TRUE)
## Print result
rk.header("Create package skeleton", parameters=list("Name", "anRpackage",
	"Directory", "."))
})
.rk.rerun.plugin.link(plugin="rkward::save_skeleton", settings="data.available=skel.func1\\nskel.func2\nforce.state=TRUE\nname.text=anRpackage\npath.selection=.", label="Run again")
.rk.make.hr()
