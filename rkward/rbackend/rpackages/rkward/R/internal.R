".rk.get.meta" <- function (x) {
	c (row.names (attr (x, ".rk.meta")), as.vector (attr (x, ".rk.meta")[[1]]), recursive=TRUE)
}

".rk.set.meta" <- function (x, l, m) {
	eval (substitute (attr (x, ".rk.meta") <<- data.frame (d=m, row.names=l)))
}

".rk.editor.opened" <- function (x) {
	if (!exists (".rk.editing")) .rk.editing <<- c ()
	.rk.editing <<- c (.rk.editing, deparse (substitute (x)))
}

".rk.editor.closed" <- function (x) {
	if (exists (".rk.editing")) .rk.editing <<- .rk.editing[.rk.editing != deparse (substitute (x))]
}

".rk.classify" <- function (x) {
	c (is.data.frame (x), is.matrix (x), is.array (x), is.list (x), is.function (x), dim (x))
}
