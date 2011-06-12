local({
## Prepare
## Compute
package.skeleton(name="anRpackage", list=c('skel.func1','skel.func2'), path=".", force= TRUE)
## Print result
rk.header("Create package skeleton", parameters=list("Name", "anRpackage",
	"Directory", "."))
})
