#' Miscellaneous utility functions
#' 
#' \code{rk.rename.in.container} renames a named object (column/element) in a
#' data.frame/list without changing its position.
#' 
#' \code{rk.make.repos.string} just creates a R statement for \code{repos}. A
#' typical user should not need to use this function.
#' 
#' \code{rk.select.CRAN.mirror} is an in-house replacement for
#' \code{\link{chooseCRANmirror}} without changing \code{options ("repos")},
#' permanently. It uses native KDE gui and provides more information on each
#' mirror.
#' 
#' @aliases rk.misc rk.rename.in.container rk.make.repos.string
#'   rk.select.CRAN.mirror
#' @param x a data.frame or list.
#' @param old_name a string, the name of the column or element to be renamed.
#' @param new_name a string, the new name.
#' @param envir an environment where \code{x} is available.
#' @return \code{rk.rename.in.container} returns \code{NULL} on successfule
#'   renaming, otherwise an error.
#' 
#' \code{rk.make.repos.string} returns a valid R expression as a character
#'   string which can then be parsed and evaluated.
#' 
#' \code{rk.select.CRAN.mirror} returns the URL of the chosen mirror, as a
#'   string.
#' @author Thomas Friedrichsmeier \email{rkward-devel@@lists.sourceforge.net}
#' @keywords attribute misc utilities
#' @rdname rk.misc
#' @examples
#' 
#' ## rk.rename.in.container
#' ir <- iris
#' str (ir)
#' rk.rename.in.container(ir, "Species", "Taxonomic.Group")
#' str (ir)
#' 
# renames a named object in a data.frame/list without changing it's position
# TODO: create a generic function instead, that can handle all kinds of renames
"rk.rename.in.container" <- function (x, old_name, new_name, envir=parent.frame()) {
	temp <- (names (x) == old_name)
	i = 1;
	for (val in temp) {
		if (val) {
			eval (substitute (names (x)[i] <- new_name), envir=envir)
			return ()
		}
		i = i+1;
	}
	error ("Could not find column with given name")
}

"rk.make.repos.string" <- function () {
	x <- getOption ("repos")
	len <- length (x)
	ret <- sprintf ("c (")
	first <- TRUE
	for (i in 1:len) {
		if (first) {
			first <- FALSE
		} else {
			ret <- sprintf ("%s, ", ret)
		}
		if (!(is.null (names (x)) || (names (x)[i] == ""))) {
			ret <- sprintf ("%s%s=\"%s\"", ret, names (x)[i], x[i])
		} else {
			ret <- sprintf ("%s\"%s\"", ret, x[i])
		}
	}
	ret <- sprintf ("%s)", ret)
	ret
}

# a wrapper around chooseCRANmirror() without changing options ("repos"), permanently
"rk.select.CRAN.mirror" <- function () {
	old_repos <- getOption("repos")
	on.exit (options (repos=old_repos))

	if (!interactive())
		stop("cannot choose a CRAN mirror non-interactively")
	m <- getCRANmirrors(all = FALSE, local.only = FALSE)
	res <- menu (paste(m[, 1L], m[, 5L], sep = " - "), getOption("menu.graphics"), "CRAN mirror")
	if (res > 0L) {
		URL <- m[res, "URL"]
		repos <- getOption("repos")
		repos["CRAN"] <- gsub("/$", "", URL[1L])
		options(repos = repos)
	}

	return (as.character (getOption ("repos")["CRAN"]))
}

#' Slightly smarter variant of old.packages()
#'
#' For most purposes, this function is identical to old.packages(). However, if the same
#' package is installed to different locations, in different versions, old.packages() will
#' treat each of these installations separately. Thus, e.g. if lib.loc == c("A", "B") and
#' package X is installed in B at an outdated version 0.1, but in A at the most recent version 0.2,
#' old.packages() will report package X at B as old. In contrast rk.old.packages() will recognize
#' that the current version is higher up in the path, and not report package X as old.
#'
#' @return a character vector of packages which are really old
#'
#' @author Thomas Friedrichsmeier \email{rkward-devel@@lists.sourceforge.net}
#' @keywords attribute misc utilities
#' @rdname rk.misc
#' @examples
#' 
#' ## NOT RUN
#' rk.old.packages()
#' 
"rk.old.packages" <- function (lib.loc = NULL, repos = getOption("repos"), contriburl = contrib.url(repos, type), instPkgs = installed.packages(lib.loc = lib.loc),
                             method, available = NULL, checkBuilt = FALSE, type = getOption("pkgType")) {
	if (is.null (lib.loc)) lib.loc <- .libPaths ()
	if (is.null (available)) available <- available.packages ()

	seen.packages <- character (0)
	old <- NULL
	for (l in lib.loc) {
		# check packages in one location at a time
		inst <- instPkgs[instPkgs[,"LibPath"] == l,]
		old <- rbind (old, 
			old.packages (lib.loc=l, repos=repos, contriburl=contriburl, instPkgs=inst, method=method, available=available, checkBuilt=checkBuilt, type=type))

		# and discard any which are masked, before looking at further locations
		seen.packages <- c (seen.packages, inst[, "Package"])
		instPkgs <- instPkgs[!(instPkgs[, "Package"] %in% seen.packages),]
	}
	old
}
