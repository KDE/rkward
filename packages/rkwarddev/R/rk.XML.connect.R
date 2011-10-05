#' Create XML node "connect" for RKWard plugins
#'
#' If you define a \code{XiMpLe.node} object as \code{governor} which is not a \code{<convert>} node
#' and \code{not=FALSE}, the function will automatically append ".state" to its \code{id}.
#'
#' @param governor Either a character string (the \code{id} of the property whose state should control
#'		the \code{client}), or an object of class \code{XiMpLe.node} (whose \code{id} will be extracted
#'		and used). Usually a \code{<convert>} node defined earlier (see
#'		\code{\link[rkwarddev:rk.XML.convert]{rk.XML.convert}}), or the ".state" value of some
#'		apropriate node.
#' @param client Character string, the \code{id} if the element to be controlled by \code{governor}.
#' @param not Logical, if \code{TRUE}, the state of \code{governor} (\code{TRUE/FALSE}) will be inversed.
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
#' @seealso
#'		\code{\link[rkwarddev:rk.XML.convert]{rk.XML.convert}},
#'		\code{\link[rkwarddev:rk.XML.external]{rk.XML.external}},
#'		\code{\link[rkwarddev:rk.XML.logic]{rk.XML.logic}}
#'		\code{\link[rkwarddev:rk.XML.set]{rk.XML.set}},
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.connect <- rk.XML.connect(governor="lgc_foobar", client="frame_bar")
#' cat(pasteXMLNode(test.connect))

rk.XML.connect <- function(governor, client, set="enabled", not=FALSE, reconcile=FALSE){

	if(length(governor) > 1 | length(client) > 1){
		stop(simpleError("'governor' and 'client' must be of length 1!"))
	} else {}

	# let's see if we need to extract IDs first
	client.id <- check.ID(client)
	governor.id <- check.ID(governor)
	# if governor is an XML node but not <convert>, append ".state"
	if(inherits(governor, "XiMpLe.node")){
		if(!identical(governor@name, "convert") & !isTRUE(not)){
			governor.id <- paste(governor.id, "state", sep=".")
		} else {}
	} else {}
	if(isTRUE(not)){
		governor.id <- paste(governor.id, "not", sep=".")
	} else {}

	attr.list <- list(governor=as.character(governor.id))

	invalid.sets <- !set %in% c("enabled", "visible", "required")
	if(length(set) > 1 | any(invalid.sets)){
		stop(simpleError(paste("Invalid value for 'set': ", set, sep="")))
	} else {
		attr.list[["client"]] <- paste(client.id, set, sep=".")
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
