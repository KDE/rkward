# check the context in which this package is loaded
#' @export
.onAttach <- function(...) {
	.rk.inside.rkward.session(warn = TRUE)
}

# this function shall test if the rkward package was loaded in a running RKWard session
#' @export
.rk.inside.rkward.session <- function(warn = FALSE){
	inside.rkward <- is.loaded("rk.do.generic.request")
	if(isTRUE(warn) & !isTRUE(inside.rkward)){
		warning("You've loaded the package 'rkward', but RKWard doesn't appear to be running. If this causes trouble, try detach(\"package:rkward\").",
		call. = FALSE)
	}
	return(inside.rkward)
}

#' @export
".rk.get.meta" <- function (x) {
	y <- attr (x, ".rk.meta");
	c (names (y), as.character (y))
}

#' @export
".rk.set.meta" <- function (x, m) {
	eval (substitute (attr (x, ".rk.meta") <<- m))
}

#' @export
".rk.set.invalid.field" <- function (x, row, value) {
	l <- attr (x, ".rk.invalid.fields");
	if (is.null (l)) l <- list ();
	l[[as.character(row)]] <- value;
	if (length (l) == 0) l <- NULL
	eval (substitute (attr (x, ".rk.invalid.fields") <<- l))
}

#' @export
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
#' @export
".rk.data.frame.delete.row" <- function (x, index) {
	attriblist <- list ()
	for (i in 1:dim (x)[2]) {
		attriblist[[names (x)[i]]] <- attributes (x[[i]])
	}
	newx <- x[-index, , drop=FALSE]
	row.names (newx) <- c (1:dim(newx)[1])
	for (i in 1:dim (newx)[2]) {
		attributes (newx[[i]]) <- attriblist[[names (newx)[i]]]
	}
	eval (substitute (x <<- newx))
}

# function below is only needed to ensure a nice ordering of the columns. Simply adding a new column would be much easier than this.
#' @export
".rk.data.frame.insert.column" <- function (x, label, index=0) {
	column <- rep (as.numeric (NA), times=dim (x)[1])
	if ((index == 0) || (index > dim (x)[2])) {	# insert column at end
		eval (substitute (x[[label]] <<- column))
	} else {
		for (i in dim (x)[2]:index) {
			eval (substitute (x[i+1] <<- x[[i]]))
			eval (substitute (names (x)[i+1] <<- names (x)[i]))
		}
		eval (substitute (x[index] <<- column))
		eval (substitute (names (x)[index] <<- label))
	}
}

#' @export
".rk.do.error" <- function () {
# comment in R sources says, it may not be good to query options during error handling. But what can we do, if R_ShowErrorMessages is not longer exported?
	if (getOption ("show.error.messages")) {
		.Call ("rk.do.error", c (geterrmessage ()));
	}
}

#' @export
".rk.set.reply" <- function (x) .rk.variables$.rk.rkreply <- x

#' @export
".rk.do.call" <- function (x, args=NULL) {
	.rk.set.reply (NULL)
	.Call ("rk.do.command", c (x, args));
	return (.rk.variables$.rk.rkreply)
}

#' @export
".rk.do.plain.call" <- function (x, args=NULL, synchronous=TRUE) {
	.Call ("rk.do.generic.request", c (x, args), isTRUE (synchronous))
}

#' @export
".rk.find.package.pluginmaps" <- function (package, all.maps=FALSE) {
	if(isTRUE(all.maps)){
		# look for all pluginmaps in the rkward folder
		pluginmaps <- sapply(package, function(this.package){
				dir(system.file("rkward", package=this.package), pattern="*.pluginmap", full.names=TRUE)
			})
	} else {
		# check if a main .pluginmap file is provided
		pluginmaps <- sapply(package, function(this.package){
				system.file(file.path("rkward", paste(this.package, ".pluginmap", sep="")), package=this.package)
			})
	}
	return(pluginmaps)
}

# Gather status information on installed and available packages.
# Return value is used in class RKRPackageInstallationStatus of the frontend
#' @export
".rk.get.package.intallation.state" <- function () {
	# fetch all status information
	available <- .rk.cached.available.packages ()
	inst <- installed.packages (fields="Title")
	old <- as.data.frame (rk.old.packages (available=available), stringsAsFactors=FALSE)
	new <- new.packages (instPkgs=inst, available=available)

	# convert info to a more suitable format
	available <- as.data.frame (available, stringsAsFactors=FALSE)
	inst <- as.data.frame (inst, stringsAsFactors=FALSE)
	oldinst <- match (paste (old$Package, old$LibPath), paste (inst$Package, inst$LibPath))	# convert package names to position with in the installed packages info
	oldavail <- match (old$Package, available$Package)	# convert package names to position with in the available packages info
	new <- match (new, available$Package)	# same for new packages

	# as a side effect, we update the list of known installed packages in the frontend
	.rk.do.plain.call ("updateInstalledPackagesList", sort (unique (as.character (inst$Package))), synchronous=FALSE)

	list ("available" = list (available$Package, available$Title, available$Version, available$Repository, grepl ("rkward", available$Enhances)),
		"installed" = list (inst$Package, inst$Title, inst$Version, inst$LibPath, grepl ("rkward", inst$Enhances)),
		"new" = as.integer (new - 1),
		"old" = list (as.integer (oldinst - 1), as.integer (oldavail - 1)),
		"repos" = as.character (options("repos")$repos))
}

# package information formats may - according to the help - be subject to change. Hence this function to cope with "missing" values
# also it concatenates everything to a single vector, so we can easily get the whole structure with a single call
#' @export
".rk.get.installed.packages" <- function () {
	x <- as.data.frame(installed.packages(fields="Title"))
	# does a package enhance RKWard, i.e. provide plugins?
	enhance.rk <- ifelse(is.na(x$Enhances), FALSE, grepl("rkward", x$Enhances))

	# as a side effect, we update the list of known installed packages in the frontend
	.rk.do.plain.call ("updateInstalledPackagesList", sort (unique (as.character (x$Package))), synchronous=FALSE)
	# check for pluginmaps only in packages which enhance RKWard
	rk.load.pluginmaps (.rk.find.package.pluginmaps(x$Package[enhance.rk]), force.add=FALSE, force.reload=FALSE)

	return(list(Package=as.character(x$Package), Title=as.character(x$Title), 
		Version=as.character(x$Version), LibPath=as.character(x$LibPath),
		EnhanceRK=as.logical(enhance.rk)))
}

# This function works like available.packages (with no arguments), but does simple caching of the result, and of course uses a cache if available. Cache is only used, if it is less than 1 hour old, and options("repos") is unchanged.
#' @export
".rk.cached.available.packages" <- function () {
	x <- NULL
	if (exists ("available.packages.cache", envir=.rk.variables) && (!is.null (.rk.variables$available.packages.cache))) {
		if (.rk.variables$available.packages.cache$timestamp > (Sys.time () - 3600)) {
			if (all (.rk.variables$available.packages.cache$repos$repos == options ("repos")$repos)) {
				x <- .rk.variables$available.packages.cache$cache
			}
		}
	}

	if (is.null(x)) {
		x <- available.packages(fields="Title")
		.rk.variables$available.packages.cache <- list (cache = x, timestamp = Sys.time (), repos = options ("repos"))
	}

	return (x)
}

#".rk.init.handlers" <- function () {
#	options (warning.expression = expression ())
#	.Internal (.addCondHands (c ("message", "warning", "error"), list (function (m) { .Call ("rk.do.condition", c ("m", conditionMessage (m))) }, function (w) { .Call ("rk.do.condition", c ("w", conditionMessage (w))) }, function (e) { .Call ("rk.do.condition", c ("e", conditionMessage (e))) }), globalenv (), NULL, TRUE))
#}

# these functions can be used to track assignments to R objects. The main interfaces are .rk.watch.symbol (k) and .rk.unwatch.symbol (k). This works by copying the symbol to a backup environment, removing it, and replacing it by an active binding to the backup location
#' @export
".rk.watched.symbols" <- new.env ()

#' @export
".rk.make.watch.f" <- function (k) {
	# we need to make sure, the functions we use are *not* looked up as symbols in .GlobalEnv.
	# else, for instance, if the user names a symbol "missing", and we try to resolve it in the
	# wrapper function below, evaluation would recurse to look up "missing" in the .GlobalEnv
	# due to the call to "if (!missing(value))".
	get <- base::get
	missing <- base::missing
	assign <- base::assign
	.rk.do.call <- rkward::.rk.do.call
	invisible <- base::invisible

	function (value) {
		if (!missing (value)) {
			assign (k, value, envir=.rk.watched.symbols)
			.Call ("rk.do.command", c ("ws", k));
#			NOTE: the above is essentially the same a
#				.rk.do.call ("ws", k);
#			only minimally faster.
			invisible (value)
		} else {
			get (k, envir=.rk.watched.symbols)
		}
	}
}

#' @export
".rk.watch.symbol" <- function (k) {
	f <- .rk.make.watch.f (k)
	.Call ("rk.copy.no.eval", k, globalenv(), .rk.watched.symbols);
	#assign (k, get (k, envir=globalenv ()), envir=.rk.watched.symbols)
	rm (list=k, envir=globalenv ())

	.rk.makeActiveBinding.default (k, f, globalenv ())

	invisible (TRUE)
}

# not needed by rkward but provided for completeness
#' @export
".rk.unwatch.symbol" <- function (k) {
	rm (list=k, envir=globalenv ())

	assign (k, .rk.watched.symbols$k, envir=globalenv ())

	rm (k, envir=.rk.watched.symbols);

	invisible (TRUE)
}

#' @export
".rk.watch.globalenv" <- function () {
	newlist <- ls (globalenv (), all.names=TRUE)
	oldlist <- ls (.rk.watched.symbols, all.names=TRUE)
	for (old in oldlist) {		# unwatch no longer present items
		if (!(old %in% newlist)) {
			rm (list=old, envir=.rk.watched.symbols);
		}
	}

	for (new in newlist) {		# watch new items
		if (!(new %in% oldlist)) {
			.rk.watch.symbol (new)
		}
	}
}

#' @export
".rk.get.vector.data" <- function (x) {
	ret <- list ();
	ret$data <- as.vector (unclass (x));
	ret$levels <- levels (x)
	if (is.null (ret$levels)) ret$levels <- ""
	i <- attr (x, ".rk.invalid.fields")
	ret$invalids <- as.character (c (names (i), i));
	if (length (ret$invalids) == 0) ret$invalids <- ""
	ret
}

# Change storage type of x to mode newmode.
# Keeps the .rk.meta attribute, and levels attributes, but the data is erased!
#' @export
".rk.set.vector.mode" <- function (x, fun, envir=parent.frame ()) {
	y <- fun(rep(NA, length.out = length(x)))

	newattrs <- attributes(y)
	if (is.null (newattrs)) newattrs <- list ()
	newattrs[[".rk.meta"]] <- attributes(x)[[".rk.meta"]]
	lvls <- attributes(x)[["levels"]]
	if (!is.null (lvls)) newattrs[["levels"]] <- lvls
	attributes(y) <- newattrs

	eval (substitute (x <- y), envir=envir)
}

#' @export
".rk.get.structure" <- function (x, name, envlevel=0, namespacename=NULL) {
	.Call ("rk.get.structure", x, as.character (name), as.integer (envlevel), namespacename)
}

#' @export
".rk.try.get.namespace" <- function (name) {
	tryCatch (asNamespace (name), error = function(e) NULL)
}

#' @export
".rk.get.structure.global" <- function (name, envlevel=0, namespacename=NULL) {
	.Call ("rk.get.structure.global", as.character (name), as.integer (envlevel), namespacename)
}

#' @export
".rk.get.slots" <- function (x) {
	slotnames <- methods::slotNames (class (x))
	ret <- lapply (slotnames, function (slotname) slot (x, slotname))
	names (ret) <- slotnames
	ret
}

#' @export
".rk.get.environment.children" <- function (x, envlevel=0, namespacename=NULL) {
	ret <- list ()

	if (envlevel < 2) {		# prevent infinite recursion
		lst <- ls (x, all.names=TRUE)
		if (is.null (namespacename)) {
			for (childname in lst) {
				ret[[childname]] <- .rk.get.structure (name=childname, envlevel=envlevel, envir=x)
			}
		} else {
			# for R 2.4.0 or greater: operator "::" works if package has no namespace at all, or has a namespace with the symbol in it
			ns <- tryCatch (asNamespace (namespacename), error = function(e) NULL)
			for (childname in lst) {
				misplaced <- FALSE
				if ((!is.null (ns)) && (!exists (childname, envir=ns, inherits=FALSE))) misplaced <- TRUE
				ret[[childname]] <- .rk.get.structure (name=childname, envlevel=envlevel, misplaced=misplaced, envir=x)
			}
		}
	}

	ret
}

# hidden, as this is not portable to different output formats
#' @export
".rk.cat.output" <- function (x) {
	cat (x, file = rk.get.output.html.file(), append = TRUE)
}

#' @export
".rk.rerun.plugin.link" <- function (plugin, settings, label) {
	.rk.cat.output (paste ("<a href=\"rkward://runplugin/", plugin, "/", URLencode (settings), "\">", label, "</a>", sep=""))
}

#' @export
".rk.make.hr" <- function () {
	.rk.cat.output ("<hr />\n");
}

# General purpose storage environment (which will hopefully never get locked by R)
#' @export
".rk.variables" <- new.env ()
assign(".rk.active.device", 1, envir=.rk.variables)
assign(".rk.output.html.file", NULL, envir=.rk.variables)
assign(".rk.rkreply", NULL, envir=.rk.variables)
assign("available.packages.cache", NULL, envir=.rk.variables)

#' @export
".rk.backups" <- new.env ()

# where masking is not enough, we need to assign in the original environment / namespace. This can only be done after package loading,
# so we have a separate function for that.
#' @export
".rk.fix.assignments" <- function () {
	## History manipulation function (overloads for functions by the same name in package utils)
	rk.replace.function ("loadhistory",  as.environment ("package:utils"),
		function (file = ".Rhistory") {
			invisible (.rk.do.plain.call ("commandHistory", c ("set", readLines (file))))
		}, copy.formals = FALSE)

	rk.replace.function ("savehistory",  as.environment ("package:utils"),
		function (file = ".Rhistory") {
			invisible (writeLines (.rk.do.plain.call ("commandHistory", "get"), file))
		}, copy.formals = FALSE)

	rk.replace.function ("timestamp",  as.environment ("package:utils"),
		function (stamp = date(), prefix = "##------ ", suffix = " ------##", quiet = FALSE) {
			stamp <- paste(prefix, stamp, suffix, sep = "")
			.rk.do.plain.call ("commandHistory", c ("append", stamp))
			if (!quiet) cat(stamp, sep = "\n")
			invisible(stamp)
		}, copy.formals = FALSE)

	## Interactive menus
	rk.replace.function ("select.list", as.environment ("package:utils"), 
		function () {
			# the "list" parameter was renamed to "choices" in R 2.11.0
			if (!exists ("list", inherits=FALSE)) list <- choices
			# the "graphics" parameter was introduced in R 2.11.0, so we cannot rely on its existance
			if (!exists ("graphics", inherits=FALSE)) graphics <- TRUE
			if (graphics) {
				return (rk.select.list (list, preselect, multiple, title))
			}

			# for text list, use the default implementation
			eval (body (.rk.backups$select.list))
		})

	rk.replace.function ("menu", as.environment ("package:utils"),
		function () {
			if (graphics) {
				res <- rk.select.list (choices, multiple=FALSE, title=title)
				return (match(res, choices, nomatch = 0L))
			}

			# for text menus, use the default implementation
			eval (body (.rk.backups$menu))
		})

	# call separate assignments functions:
	if (exists (".rk.fix.assignments.graphics")) eval (body (.rk.fix.assignments.graphics)) # internal_graphics.R
}
