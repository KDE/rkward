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
	type <- 0
	if (is.data.frame (x)) type = type + 1
	if (is.matrix (x)) type = type + 2
	if (is.array (x)) type = type + 4
	if (is.list (x)) type = type + 8
	if (type != 0) type = type + 16 else type = 32
	if (is.function (x)) type = type + 128
	if (!is.null (attr (x, ".rk.meta"))) type = type + 256
	c (type, dim (x))
}

".rk.get.type" <- function (x) {
	if (is.data.frame (x) || is.matrix (x) || is.array (x) || is.list (x)) return (1)		# container
	if (is.function (x)) return (2)		# function
	if (is.vector (x)) return (3)		# a vector/variable
	return (4)		# something else
}
