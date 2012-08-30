#' Various label related utility functions
#' 
#' \code{rk.get.label} retrieves the rkward label (if any) of the given object.
#' 
#' \code{rk.set.label} sets the rkward label for the given object.
#'
#' \code{rk.list.labels} retrieves the rkward labels for a list of objects.
#' Most importantly, this can be used for extracting all column labels in a
#' \code{data.frame}, conveniently. The parameter \code{fill} controls, what
#' happens, when no rkward labels have been assigned. The default (\code{FALSE})
#' is to return empty strings for any missing labels. For \code{fill=TRUE}, missing
#' labels will be filled with the short names of the object. You can also pass
#' a character vector of default labels to use as the \code{fill} parameter.
#'
#' \code{rk.get.short.name} creates a short name for the given object.
#' 
#' \code{rk.get.description} creates descriptive string(s) for each of the
#' arguments in "\code{\dots{}}"; collapsing into a single string using
#' \code{paste.sep} (if not NULL). If \code{is.substitute=TRUE}, the arguments
#' will be deparsed, first, which can be useful when using
#' \code{rk.get.description} inside a function.
#' 
#' \code{rk.list.names} returns the names of the arguments passed as
#' \code{...}; when using \code{rk.list.names} inside a function, it may be
#' necessary to increase the \code{deparse.level} level.
#' 
#' \code{rk.list} returns a list of its arguments, with \code{names} set as
#' returned by \code{rk.get.description()}. This can be used as a drop-in
#' replacement for \code{\link{list}}.
#' 
#' @aliases rk.get.label rk.set.label rk.get.short.name rk.get.description
#'   rk.list.names rk.list
#' @param x any R object
#' @param label a string, to set the label attribute of an object
#' @param envir an environment, where the attribute is evaluated
#' @param fill a logical or character. See Details.
#' @param paste.sep a string, used as the \code{collapse} argument for paste
#' @param is.substitute a logical (not NA). See Details.
#' @return \code{rk.set.label} returns the result of the evaluation of "setting
#'   the label" while the others return a character vector.
#' @author Thomas Friedrichsmeier \email{rkward-devel@@lists.sourceforge.net}
#' @keywords utilities
#' @rdname rk.label
#' @examples
#' 
#' x <- data.frame(a=c(1:3), b=c(2:4))
#' rk.set.label(x[["a"]], "First column")
#' rk.get.short.name (x$a)                   # "x$a"
#' rk.get.label (x$a)                        # "First column"
#' rk.get.description (x$a)                  # "x$a (First column)"
#' rk.list.labels (x)                        # "First column" ""
#' rk.list.labels (x, TRUE)                  # "First column" "b"
#' rk.list.names (x, x$a, x$b)               # "x" "x$a" "x$b"
#' names (rk.list (x$a, x$b))                # "x$a (First column)" "x$b"
#' 

# retrieve the rkward label (if any) of the given object
#' @export
"rk.get.label" <- function (x) {
	if (is.call (x) || is.name (x)) {
		x <- eval (x)
	}
	ret <- attr (x, ".rk.meta")[["label"]]
	if (is.null (ret) || is.na (ret) || (length (ret) < 1)) ""
	else as.character (as.vector (ret))[1L]
}

# set rkward label
#' @rdname rk.label
#' @export
"rk.set.label" <- function (x, label, envir=parent.frame()) {
	if (is.call (x) || is.name (x)) {
		meta <- attr (eval (x, envir=envir), ".rk.meta")
	} else {
		meta <- attr (x, ".rk.meta")
	}
	meta[["label"]] <- as.character (label)[1L]
	eval(substitute(attr(x, ".rk.meta") <- meta), envir = envir)
}

# retrieve the rkward labels for items in the given list
#' @rdname rk.label
#' @export
"rk.list.labels" <- function (x, fill=FALSE) {
	ret <- sapply (x, rk.get.label)
	if (isTRUE (fill)) {
		ret[ret == ""] <- names(x)[ret == ""]
	} else if (!is.logical (fill)) {
		ret[ret == ""] <- as.character (fill)[ret == ""]
	}
	ret
}

# get a short name for the given object
#' @rdname rk.label
#' @export
"rk.get.short.name" <- function (x) {
	if (is.call (x) || is.name (x)) {
		.rk.make.short.name (deparse (x))
	} else {
		.rk.make.short.name (deparse (substitute (x)))
	}
}

# make a short name from the given arg (a character string)
# e.g. return "b" for a[["b"]] (but 'a::"b"' for a::"b"
#' @export
".rk.make.short.name" <- function (x) {
	splt <- strsplit (x, "[[\"", fixed=TRUE)[[1]]
	spltlen <- length (splt)
	if (spltlen == 1) {
		splt[1]
	} else {
		strsplit (splt[spltlen], "\"]]", fixed=TRUE)[[1]][1]
	}
}

# get descriptive strings for each of the arguments in ...
#' @rdname rk.label
#' @export
"rk.get.description" <- function (..., paste.sep=NULL, is.substitute=FALSE) {
	args <- list(...)
	if (is.substitute) {
		argnames <- list ()
		j <- 1
		for (symb in list (...)) {
			argnames[j] <- deparse (symb)
			j <- j + 1
		}
	} else {
		argnames <- rk.list.names (...)
	}
	descript <- c ()

	for (i in 1:length (args)) {
		lbl <- rk.get.label (args[[i]])
		if (is.substitute) {
			shortname <- .rk.make.short.name (as.character (argnames[i]))
		} else {
			shortname <- .rk.make.short.name (argnames[i])
		}

		if (is.null (lbl) || (length (lbl) != 1) || (lbl == "")) descript[i] <- shortname
		else descript[i] <- paste (shortname, " (", lbl, ")", sep="")
	}

	if (is.null (paste.sep)) {
		descript
	} else {
		paste (descript, collapse=paste.sep)
	}
}

# Drop-in replacement for list(). Returns a list of the given arguments, but with names set according to rk.get.description
#' @rdname rk.label
#' @export
"rk.list" <- function (...) {
	ret <- list (...)
	names (ret) <- rk.get.description (...)
	ret
}

# this is basically copied from R-base table (). Returns the arguments passed to ... as a character vector
#' @rdname rk.label
#' @export
"rk.list.names" <- function(..., deparse.level=2) {
	l <- as.list(substitute(list(...)))[-1]
	nm <- names(l)
	fixup <- if (is.null(nm)) 
		seq(along = l)
	else nm == ""
	dep <- sapply(l[fixup], function(x) switch(deparse.level + 1, "", if (is.symbol(x)) as.character(x) else "", deparse(x)[1]))
	if (is.null(nm)) 
		dep
	else {
		nm[fixup] <- dep
		nm
	}
}
