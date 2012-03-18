#' Create XML node "connect" for RKWard plugins
#'
#' If you define a \code{XiMpLe.node} object as \code{governor} which is not a \code{<convert>} node
#' and \code{not=FALSE}, the function will automatically append  to its \code{id}.
#' 
#' @note To get a list of the implemented modifiers in this package, call \code{rkwarddev:::all.valid.modifiers}.
#'
#' @param governor Either a character string (the \code{id} of the property whose state should control
#'		the \code{client}), or an object of class \code{XiMpLe.node} (whose \code{id} will be extracted
#'		and used). Usually a \code{<convert>} node defined earlier (see
#'		\code{\link[rkwarddev:rk.XML.convert]{rk.XML.convert}}).
#' @param client Character string, the \code{id} if the element to be controlled by \code{governor}.
#' @param get Character string, a valid modifier for the node property of \code{governor}, often
#'		the ".state" value of some apropriate node.
#' @param set Character string, a valid modifier for the node property of \code{client}, usually
#'		one of \code{"enabled"}, \code{"visible"} or \code{"required"}.
#' @param not Logical, if \code{TRUE}, the state of \code{governor} (\code{TRUE/FALSE}) will be inversed.
#' @param reconcile Logical, forces the \code{governor} to only accept values which are valid for
#'		the \code{client} as well.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\code{\link[rkwarddev:rk.XML.convert]{rk.XML.convert}},
#'		\code{\link[rkwarddev:rk.XML.external]{rk.XML.external}},
#'		\code{\link[rkwarddev:rk.XML.logic]{rk.XML.logic}}
#'		\code{\link[rkwarddev:rk.XML.set]{rk.XML.set}},
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.connect <- rk.XML.connect(governor="lgc_foobar", client="frame_bar")
#' cat(pasteXML(test.connect))

rk.XML.connect <- function(governor, client, get="state", set="enabled", not=FALSE, reconcile=FALSE){

	if(length(governor) > 1 | length(client) > 1){
		stop(simpleError("'governor' and 'client' must be of length 1!"))
	} else {}

	# let's see if we need to extract IDs first
	client.id <- check.ID(client)
	governor.id <- check.ID(governor)
	# if governor is an XML node but not <convert>, append ".state"
	if(inherits(governor, "XiMpLe.node")){
		node.name <- slot(governor, "name")
		if(!identical(node.name, "convert")){
			# validate get modifier
			if(modif.validity(governor, modifier=get)){
				governor.id <- paste(governor.id, get, sep=".")
			} else {}
		} else {}
	} else {}
	if(isTRUE(not)){
		governor.id <- paste(governor.id, "not", sep=".")
	} else {}

	attr.list <- list(governor=as.character(governor.id))

	# validate set modifier
	if(modif.validity(client, modifier=set, warn.only=FALSE)){
		attr.list[["client"]] <- paste(client.id, set, sep=".")
	} else {}

	if(isTRUE(reconcile)){
			attr.list[["reconcile"]] <- "true"
	} else {}

	node <- XMLNode("connect", attrs=attr.list)

	return(node)
}
