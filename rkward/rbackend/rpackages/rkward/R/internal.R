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

".rk.data.frame.insert.row" <- function (x, index=0) {
	if ((index == 0) || (index > dim (x)[1])) {	# insert row at end
		eval (substitute (x[dim(x)[1]+1,] <<- c (NA)))
	} else {
		for (i in dim (x)[1]:index) {
			eval (substitute (x[i+1,] <<- x[i,]))
		}
		eval (substitute (x[index,] <<- c (NA)))
	}
}

".rk.data.frame.delete.row" <- function (x, index) {
	for (i in index:dim (x)[1]) {
		eval (substitute (x[i,] <<- x[i+1,]))
	}
	# TODO: is there a nice way to get rid of the row entirely (and without removing the meta-data)?
	eval (substitute (x[dim (x)[1],] <<- c (NA)))
}

# function below is only needed to ensure a nice ordering of the columns. Simply adding a new column would be much easier than this.
".rk.data.frame.insert.column" <- function (x, label, index=0) {
	if ((index == 0) || (index > dim (x)[2])) {	# insert column at end
		eval (substitute (x[[label]] <<- c (NA)))
	} else {
		for (i in dim (x)[2]:index) {
			eval (substitute (x[i+1] <<- x[[i]]))
			eval (substitute (names (x)[i+1] <<- names (x)[i]))
		}
		eval (substitute (x[index] <<- c (NA)))
		eval (substitute (names (x)[index] <<- label))
	}
}

