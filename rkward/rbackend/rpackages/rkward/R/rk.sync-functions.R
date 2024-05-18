# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
#' Sync R object(s)
#' 
#' RKWard keeps an internal representation of objects in the R workspace. For
#' objects in the \code{.GlobalEnv}, this representation is updated after each
#' top-level statement. For the rare cases where this is not enough,
#' \code{rk.sync} can be used to update the representation of a single object,
#' \code{x}, while \code{rk.sync.global} scans the \code{.GlobalEnv} for new
#' and removed objects, and updates as appropriate.
#' 
#' These functions are rarely needed outside automated testing. However,
#' rk.sync() can be useful, if an object outside the \code{.GlobalEnv} has
#' changed, since this will not be detected automatically. Also, by default
#' RKWard does not recurse into environments when updating its representation
#' of objects. rk.sync() can be used, here, to inspect the objects inside
#' environments (see examples).
#' 
#' @aliases rk.sync rk.sync.global
#' @param x any R object to sync
#' @return \code{NULL}, invisibly.
#' @author Thomas Friedrichsmeier \email{rkward-devel@@kde.org}
#' @seealso \url{rkward://page/rkward_workspace_browser}
#' @keywords utilities misc
#' @rdname rk.sync
# should this really be public?
#' @export
#' @examples
#' \dontrun{
#' rk.sync (rkward::rk.record.plot)
#' }
"rk.sync" <- function (x) {
	object <- deparse (substitute (x))
	.rk.call.nested("sync", list(NULL, NULL, object))
}

# should this really be public?
#' @export
#' @rdname rk.sync
"rk.sync.global" <- function () {
	.rk.call.nested("sync", rk.check.env.changes(globalenv()))
}
