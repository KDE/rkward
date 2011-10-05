#' Create XML node convert for RKWard plugins
#'
#' The recognized property names for \code{sources} are the following:
#' \code{string}, \code{state}, \code{text}, \code{selected}, \code{root},
#' \code{available}, \code{source}, \code{number}, \code{enabled}, \code{checked}, \code{selection},
#' \code{parent}, \code{objectname}, \code{active}, \code{int}, \code{real}, \code{model},
#' \code{table}, \code{labels}, \code{fixed_factors}, \code{dependent} and \code{code}.
#' They are not globally valid for all XML elements, see the section on "Properties of plugin elements"
#' to see which is useful for what tag. If \code{sources} holds \code{XiMpLe.node}
#' objects, the validity of properties is automatically checked for that tag.
#'
#' @param sources A list with at least one value, either resembling the \code{id} of
#'		an existing element to be queried as a character string, or a previously defined object
#'		of class \code{XiMpLe.node} (whose \code{id} will be extracted and used). If you want
#'		to examine e.g. the state or string value specificly, just name the value accoringly, e.g.,
#'		\code{sources=list("vars0", string="input1", state="chkbx2")}.
#' @param mode A named vector with either exactly one of the following elements:
#'		\itemize{
#'			\item{\code{equals}}{True if \code{sources} equals this value.}
#'			\item{\code{notequals}}{True if \code{sources} differs from this value.}
#'			\item{\code{and}}{True if all \code{sources} are true. The sources must be boolean,
#'				and the actual value here is irrelevant, so \code{mode=c(and="")} is valid.}
#'			\item{\code{or}}{True if any of the \code{sources} is true.  The sources must be boolean,
#'				and the actual value here is irrelevant, so \code{mode=c(or="")} is valid.}
#'		}
#'		or at least one of these elemets:
#'		\itemize{
#'			\item{\code{min}}{True if \code{sources} is at least this value. They must be numeric.}
#'			\item{\code{max}}{True if \code{sources} is below this value. They must be numeric.}
#'		}
#' @param required Logical, sets the state of the \code{required_true} attribute. If \code{TRUE},
#'		the plugin submit button is only enabled if this property is true.
#' @param id.name Character string, a unique ID for this plugin element.
#'		If \code{"auto"}, an ID will be generated automatically from the \code{sources}
#'		and \code{mode} value.
#' @return A list of objects of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\code{\link[rkwarddev:rk.XML.connect]{rk.XML.connect}},
#'		\code{\link[rkwarddev:rk.XML.external]{rk.XML.external}},
#'		\code{\link[rkwarddev:rk.XML.logic]{rk.XML.logic}},
#'		\code{\link[rkwarddev:rk.XML.set]{rk.XML.set}},
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.convert <- rk.XML.convert(c(string="foo"), mode=c(notequals="bar"))
#' cat(pasteXMLNode(test.convert))

rk.XML.convert <- function(sources, mode=c(), required=FALSE, id.name="auto"){

	# check the mode element
	mode.name <- names(mode)
	if(length(mode) < 1 | length(mode.name) < 1 | length(mode) > 2 | length(mode.name) > 2){
		stop(simpleError("'mode' must have one (or two, if its a range) named element!"))
	} else {}
	if(!all(mode.name %in% c("equals","notequals","and","or","min","max"))){
		stop(simpleError(paste("The mode you provided is invalid: ", mode.name, sep="")))
	} else {}
	if(length(mode) == 2 & !all(mode.name %in% c("min","max"))){
		stop(simpleError("If 'mode' has two elements, they can only be \"min\" and \"max\"!"))
	} else {}

	if(identical(id.name, "auto")){
		sourceValsForID <- sapply(sources, function(this.source){
					return(check.ID(this.source))
			})
		attr.list <- list(id=auto.ids(paste(paste(sourceValsForID, collapse=""), mode, sep=""), prefix=ID.prefix("logic")))
	} else if(!is.null(id.name)){
		attr.list <- list(id=id.name)
	} else {
		stop(simpleError("'id.name' must have a value!"))
	}

	# firstly, check the sources. if some are named, contruct proper values
	# for RKWard, like string="foo" should actually be "foo.string"
	src.names <- names(sources)
	if(!is.null(src.names)){
		# check these names if they're valid properties here
		invalid.names <- !src.names %in% c("", "string", "state", "text", "selected", "root",
			"available", "source", "number", "enabled", "checked", "selection", "parent",
			"objectname", "active", "int", "real", "model", "table", "labels",
			"fixed_factors", "dependent", "code")
		if(any(invalid.names)){
			warning(paste("Some of the property names you provided are invalid and were ignored: ",
				paste(src.names[invalid.names], collapse=", "), sep=""))
				src.names[invalid.names] <- ""
		} else {}
		sources <- as.character(sapply(1:length(src.names), function(src.no){
				this.prop <- src.names[src.no]
				valid.prop <- prop.validity(source=sources[[src.no]], property=this.prop, bool=FALSE)
				if(nchar(valid.prop) > 0){
					new.value <- paste(check.ID(sources[[src.no]]), this.prop, sep=".")
				} else {
					new.value <- check.ID(sources[[src.no]])
				}
				return(new.value)
			}))
	} else {}

	attr.list[["sources"]] <- paste(sources, collapse=";")

	if(identical(mode.name,"equals")){
		attr.list[["mode"]] <- mode.name
		attr.list[["standard"]] <- as.character(mode)
	} else if(identical(mode.name,"notequals")){
		attr.list[["mode"]] <- mode.name
		attr.list[["standard"]] <- as.character(mode)
	} else if(identical(mode.name,"and")){
		attr.list[["mode"]] <- mode.name
	} else if(identical(mode.name,"or")){
		attr.list[["mode"]] <- mode.name
	} else {
		if("min" %in% mode.name){
			attr.list[["mode"]] <- "range"
			attr.list[["min"]] <- as.numeric(mode["min"])
		} else {}
		if("max" %in% mode.name){
			attr.list[["mode"]] <- "range"
			attr.list[["max"]] <- as.numeric(mode["max"])
		} else {}
	}

	if(isTRUE(required)){
			attr.list[["required_true"]] <- "true"
	} else {}

	node <- new("XiMpLe.node",
			name="convert",
			attributes=attr.list
		)

	return(node)
}
