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
		eval (substitute (row.names (x) <<- c (1:dim(x)[1])))
		eval (substitute (x[index,] <<- c (NA)))
	}
}

# TODO: is there a nice way to get rid of a row without removing the meta-data?
".rk.data.frame.delete.row" <- function (x, index) {
	attriblist <- list ()
	for (i in 1:dim (x)[2]) {
		attriblist[[names (x)[i]]] <- attr (x[[i]], ".rk.meta")
	}
	eval (substitute (x <<- x[-index,]))
	eval (substitute (row.names (x) <<- c (1:dim(x)[1])))
	for (i in 1:dim (x)[2]) {
		eval (substitute (attr (x[[i]], ".rk.meta") <<- attriblist[[names (x)[i]]]))
	}
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

".rk.do.call" <- function (x) {
	cat (x, file=.rk.socket)
	res <- array ()
	i <- 1
	done <- FALSE
	while (!done) {
		inp <- readLines (.rk.socket, 1)
		if (length (inp)) {
			if (inp == "#RKEND#")  {
				done <- TRUE
			} else {
				res[i] <- inp;
			}
		}
	}
	return (res)
}

".rk.test.connection" <- function (x) {
	if (.rk.do.call ("checkconnection\n") != "ok") stop ("connection to RKWard failed")
}

# package information formats may - according to the help - be subject to change. Hence this function to cope with "missing" values
# also it concatenates everything to a single vector, so we can easily get the whole structure with a single call
".rk.get.installed.packages" <- function () {
	x <- as.data.frame (installed.packages ())
	try (titles <- as.data.frame (library ()$results)$Title)
	if (length (titles) != dim (x)[1]) titles <- rep ("", dim (x)[1])
	return (c (as.vector (x$Package), as.vector (titles), as.vector (x$Version), as.vector (x$LibPath)))
}

