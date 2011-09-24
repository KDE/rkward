#' Create XML node "connect" for RKWard plugins
#'
#' @param governor Character string, the \code{id} if the property whose state should control
#'		the \code{client}. Usually a \code{<convert>} node defined earlier (see
#'		\code{\link[rkwarddev:rk.XML.convert]{rk.XML.convert}}), or the ".state" value of some
#'		apropriate node.
#' @param client Character string, the \code{id} if the element to be controlled by \code{governor}.
#' @param set Character string, one of the following values:
#'		\itemize{
#'			\item{\code{"enabled"}}{If \code{governor} becomes true, \code{client} is enabled.}
#'			\item{\code{"visible"}}{If \code{governor} becomes true, \code{client} is visible.}
#'			\item{\code{"required"}}{If \code{governor} becomes true, \code{client} is required.}
#'		}
#' @param reconcile Logical, forces the \code{governor} to only accept values which are valid for
#'		the \code{client} as well.
#' @return A list of objects of class \code{XiMpLe.node}.
#' @export
#' @examples
#' test.connect <- rk.XML.connect(governor="lgc_foobar", client="frame_bar")
#' cat(pasteXMLNode(test.connect, shine=1))

rk.XML.connect <- function(governor, client, set="enabled", reconcile=FALSE){

	if(length(governor) > 1 | length(client) > 1){
		stop(simpleError("'governor' and 'client' must be of length 1!"))
	} else {}

	attr.list <- list(governor=as.character(governor))

	invalid.sets <- !set %in% c("enabled", "visible", "required")
	if(length(set) > 1 | any(invalid.sets)){
		stop(simpleError(paste("Invalid value for 'set': ", set, sep="")))
	} else {
		attr.list[["client"]] <- paste(client, set, sep=".")
	}

	if(isTRUE(reconcile)){
			attr.list[["reconcile"]] <- "true"
	} else {}

	node <- new("XiMpLe.node",
			name="connect",
			attributes=attr.list
		)

	return(node)
}
