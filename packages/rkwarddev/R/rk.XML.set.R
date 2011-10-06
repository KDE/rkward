#' Create XML node "set" for RKWard plugins
#'
#' @param id Either a character string (the \code{id} of the property whose value should be set),
#'		or an object of class \code{XiMpLe.node} (whose \code{id} will be extracted and used).
#' @param set Character string, can be one of the following values:
#'		\itemize{
#'			\item{\code{"enabled"}}{Set the \code{id} enabled state.}
#'			\item{\code{"visible"}}{Set the \code{id} visibility state.}
#'			\item{\code{"required"}}{Set the \code{id} requirement state.}
#'		}
#' @param to Character string or logical, the value the property should be set to.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\code{\link[rkwarddev:rk.XML.convert]{rk.XML.convert}},
#'		\code{\link[rkwarddev:rk.XML.connect]{rk.XML.connect}},
#'		\code{\link[rkwarddev:rk.XML.external]{rk.XML.external}},
#'		\code{\link[rkwarddev:rk.XML.logic]{rk.XML.logic}},
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.set <- rk.XML.set(id="input_foo", set="required", to=TRUE)
#' cat(pasteXMLNode(test.set))

rk.XML.set <- function(id, set=NULL, to){

	if(length(id) > 1 | length(to) > 1){
		stop(simpleError("'id' and 'to' must be of length 1!"))
	} else {}

	# let's see if we need to extract IDs first
	prop.id <- check.ID(id)

	if(!is.null(set)){
		if(!set %in% c("state","string","enabled","required","visible")){
			stop(simpleError(paste("The 'set' property you provided is invalid: ", set, sep="")))
		} else {
			prop.id <- paste(prop.id, set, sep=".")
		}
	} else {}

	attr.list <- list(id=as.character(prop.id))

	if(is.logical(to)){
		attr.list[["to"]] <- ifelse(isTRUE(to), "true", "false")
	} else {
		attr.list[["to"]] <- as.character(to)
	}

	node <- new("XiMpLe.node",
			name="set",
			attributes=attr.list
		)

	return(node)
}
