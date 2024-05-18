# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
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
#' @author Thomas Friedrichsmeier \email{rkward-devel@@kde.org}
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
#' @export
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
	stop ("Could not find column with given name")
}

#' @export
#' @rdname rk.misc
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
#' @importFrom utils getCRANmirrors menu
#' @export
#' @rdname rk.misc
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
#' For most purposes, this function is identical to \code{\link[utils:old.packages]{old.packages()}}. However, if the same
#' package is installed to different locations, in different versions, old.packages() will
#' treat each of these installations separately. Thus, e.g. if lib.loc == c("A", "B") and
#' package X is installed in B at an outdated version 0.1, but in A at the most recent version 0.2,
#' old.packages() will report package X at B as old. In contrast rk.old.packages() will recognize
#' that the current version is higher up in the path, and not report package X as old.
#'
#' @inheritParams utils::old.packages
#' @return a character vector of packages which are really old
#'
#' @author Thomas Friedrichsmeier \email{rkward-devel@@kde.org}
#' @keywords attribute misc utilities
#' @rdname rk.old.packages
#' @examples
#' \dontrun{
#' rk.old.packages()
#' }
#' @importFrom utils contrib.url installed.packages old.packages
#' @export
"rk.old.packages" <- function (lib.loc = NULL, repos = getOption("repos"), contriburl = contrib.url(repos, type), instPkgs = installed.packages(lib.loc = lib.loc),
                             method, available, checkBuilt = FALSE, type = getOption("pkgType")) {
	if (is.null (lib.loc)) lib.loc <- .libPaths ()
	# for the time being, translate NULL into missingness and throw a warning
	if(!missing(available)){
		if (is.null (available)) {
			warning("Deprecated: available = NULL, leave missing if unused!")
			available <- substitute()
		}
	}
	if (missing (available)) available <- available.packages (contriburl=contriburl, method=method)

	seen.packages <- character (0)
	old <- NULL
	for (l in lib.loc) {
		# check packages in one location at a time
		inst <- instPkgs[instPkgs[,"LibPath"] == l, , drop=FALSE]
		old <- rbind (old, 
			old.packages (lib.loc=l, repos=repos, contriburl=contriburl, instPkgs=inst, method=method, available=available, checkBuilt=checkBuilt, type=type))

		# and discard any which are masked, before looking at further locations
		seen.packages <- c (seen.packages, inst[, "Package"])
		instPkgs <- instPkgs[!(instPkgs[, "Package"] %in% seen.packages), , drop=FALSE]
	}
	old
}


#' Start recording commands that are submitted from RKWard to R
#' 
#' To stop recording, supply NULL or "" as filename.
#' Currently used for the purpose of automated testing, only. Perhaps in the future
#' this or a similar mechanism could also be added as a user feature.
#' 
#' @param filename filename to write to (file will be truncated!).
#' @param include.all By default, some types of command are filtered (internal synchronisation commands, and run again links). Should these be included?
#' @export
#' @rdname rk.record.commands
"rk.record.commands" <- function (filename, include.all = FALSE) {
	if (is.null (filename)) filename = ""

	# NOTE: Passing params as flat character vector for purely historical reasons. This could be changed.
	res <- .rk.call("recordCommands", c(as.character(filename), if (include.all) "include.all" else "normal"))

	if (!length (res)) invisible (TRUE)
	else {
		warning (res)
		invisible (FALSE)
	}
}

#' Switch language / translation to use in the frontend.
#'
#' This feature is mostly intended for the purpose of automated testing, which needs a
#' defined language to work. It might also be useful for translators, or e.g. to look up
#' some terms untranslated in special cases where you are more familiar with the English terms than
#' your native language terms. Note that this will only strings that are translated after the call., only those which get
#' translated after the call. Most new dialogs you open, and importantly new plugin dialogs should
#' show strings in the new lanuage, however.
#' 
#' To change the language in the backend, use \code{Sys.setenv(LANGUAGE=...)} or \code{Sys.setlocale()}.
#'
#' @param LANG language code to use. "C" for no translation, i.e. generally English
#'
#' @export
#' @rdname rk.switch.frontend.language
"rk.switch.frontend.language" <- function(LANG="C") {
   .rk.call("switchLanguage", as.character(LANG))
}

#' Add one or more paths to the filesystem search path used in this session
#'
#' Add the given path to the "PATH" environment variable of the running R session. This
#' can be useful to make sure external binaries are found by Sys.which. Paths are normalized
#' before being added, and duplicates are stripped from the path.
#'
#' @param add Paths to add. May be missing, in which case the path will no be touched.
#'
#' @return A vector of the directories in the file sytem path after the adjustment
#'
#' @export
"rk.adjust.system.path" <- function (add) {
	if (!missing (add)) {
		oldpath <- unlist (strsplit(Sys.getenv("PATH"), .Platform$path.sep))
		newpath <- paste (unlist (unique (c (oldpath, normalizePath (add)))), collapse=.Platform$path.sep)
		Sys.setenv("PATH"=newpath)
	}

	# return
	unlist (strsplit(Sys.getenv("PATH"), .Platform$path.sep))
}
