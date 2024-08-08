# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
#' Class for adding menu items from R.
#' @name rk.menu
#'
#' @description Allows to add (and subsequently remove) menu items associated to specific plain R functions in the RKWard main window.
#'              Note that this mechanism is primarily targetted as users looking for an easy way to add their own customizations, or for bridiging to
#'              third party UI packages. For anything more complex, and/or specifically targetted at RKWard, it is seriously recommended to create regular
#'              RKWard plugins, for best UI consistency.
#'
#'              The reference class \code{rk.menu()} creates a handle for a menu item (which may either represent a submenu, or an action/leaf item). Handles
#'              are identified by their "path", with is a character vector, with each element identifying a level of the menu-hierarchy. (Sub-)handles can be
#'              created using the \code{$item()} method.
#'
#'              To actually create a menu entry for a handle, the method \code{$define} needs to be called, specifying a label, and - for leaf items - and
#'              associated R function.
#'
#'              Adding/removing menu items is a fairly computation heavy exercise, internally, and is handled asynchronously, in the frontend. Should you need
#'              to remove and re-add certainly elements, frequently, hiding them will be more efficient (see \code{$enable()}). Note: A disabled menu item
#'              can still be called programmatically, using \code{$call()}
#'
#'              This interface is still somewhat experimental, and currently kept to a minimal set of functions, deliberately. Please don't hesistate to give
#'              us feedback on what you would like to see added. Only items defined using this mechanism can be manipulated / removed.
#'
#' @param ... Path elements (character) given either as separate arguments, or as a multi-element character vector.
#'
#' @param label Label of the menu entry (character)
#'
#' @param func Function to call for leaf item
#'
#' @returns \code{rk.menu()} and \code{$item()} return a handle. \code{$define() returns the handle it was given (to allow command chaining)}.
#'          \code{call()} passes on the return value of the associated function. The other methods return \code{NULL}
#'
#' @import methods
#' @export rk.menu
#'
#' @examples
#' \dontrun{
#' x <- rk.menu()$item("analysis")                          # handle to the predefined analysis menu
#' sub <- x$item("my_functions")$define("My Functions")     # create submenu
#' a <- rk.menu()$item("analysis", "my_functions", "yeah")  # handle to an item in that submenu
#' a <- sub$item("yeah")                                    # alternative variant for retrieving the above handle
#' a$define("Print Yeah", function() { rk.print("Yeah!") }) # define leaf item
#' a$call()                                                 # invoke, programmatically
#' sub$remove()                                             # remove submenu, including the "yeah" action
#' }
rk.menu <- setRefClass("rk.menu",
	fields=list(path="character"),  # for future expansion: context="character" for x11 / script / data functions
	methods=list(item=function(...) {
		"Return a child item of this item, given a relative path"
		rk.menu(path=c(path, ...))
	},
	define=function(label, func) {
		"(Re-)define the menu item at this path. If call is specified, this become a leaf item, associated with the given function, otherwise, a submenu is created."
		x <- .retrieve(TRUE)
		x$label <- label
		if (!missing(func)) {
			if (exists("children", envir=x)) stop(paste("Submenu", paste(path, collapse="/"), "cannot be redefined as a leaf item."))
			x$fun <- func
		}
		.rk.call.async("menuupdate", rk.menu()$.list())
		invisible(rk.menu(path=path))
	},
	call=function() {
		"Call the function associated with this menu item"
		x <- .retrieve(FALSE)
		x$fun()
	},
	remove=function() {
		"Remove any registered menu entry at this path from the menu"
		parent <- rk.menu(path=path[1:length(path)-1])$.retrieve(FALSE)
		parent$children[[path[length(path)]]] <- NULL
		.rk.call.async("menuupdate", rk.menu()$.list())
		invisible(NULL)
	},
	enable=function(enable=TRUE, show=TRUE) {
		"Disable and/or hide this menu item"
		.retrieve(FALSE) # Check for existence
		.rk.call.async("menuenable", list(path, isTRUE(enable), isTRUE(show)))
		invisible(NULL)
	},
	.retrieve=function(create=FALSE) {
		"Internal, do not use: Walk the tree of registered menu entries. If create is set to TRUE, missing paths are created, otherwise, missing paths are an error."
		if (!exists("menuroot", envir=.rk.variables)) .rk.variables$menuroot <- new.env()
		parent <- .rk.variables$menuroot
		walkedpath <- NULL
		for(p in path) {
			walkedpath <- c(walkedpath, p)
			if (!exists("children", envir=parent)) {
				if (!create) stop(paste("Menu path", paste(walkedpath, collapse="/"), "is not defined"))
				if (exists("fun", envir=parent)) stop(paste("Leaf menu item", paste(walkedpath, collapse="/"), "cannot be redefined as a submenu."))
				parent$children <- list()
			}
			if (is.null(parent$children[[p]])) {
				if (!create) stop(paste("Menu path", paste(walkedpath, collapse="/"), "is not defined"))
				parent$children[[p]] <- new.env()
			}
			parent = parent$children[[p]]
		}
		parent
	},
	.list=function(x, id="") {
		"Internal, do not use: Create nested list representation of the menu structure."
		if (missing(x)) x <- .retrieve(FALSE)
		ret <- list()
		ret[[".ID"]] <- id
		nms <- names(x$children)
		ret[[".CHILDREN"]] <- lapply(nms, function(X) .list(x$children[[X]], X))
		ret[[".LABEL"]] <- ifelse(is.null(x$label), "", x$label)
		ret[[".FUN"]] <- ifelse(is.null(x$fun), "0", "1")
		ret
	})
)
