local({
## Compute
package.skeleton(name="anRpackage", list=c('skel.func1','skel.func2'), path="PATH", force= TRUE)
## Print result
rk.header ("Create package skeleton", parameters=list("Package name"="anRpackage",
	"Directory"="PATH"))
})
