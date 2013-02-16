#' Create XML node "switch" for RKWard plugins
#'
#' This node can only be used in \code{<logic>} sections. If the provided property
#' is logical, you must also set values for \code{true} and \code{false}. If not,
#' you must provide at least one \code{case}.
#' 
#' The values to be returned can be either \code{fixed_value} or \code{dynamic_value}.
#' A \code{fixed_value} must be a character string which will be returned if the condition
#' is met. Whereas a \code{dynamic_value} is the \code{id} of another property, an can
#' be provided as either a character string or an object of class \code{XiMpLe.node}.
#'
#' @param condition Either a character string (the \code{id} of the property whose
#'		state should be queried), or an object of class \code{XiMpLe.node}
#'		(whose \code{id} will be extracted and used).
#' @param modifier Character string, an optional modifier to be appended tp \code{condition}.
#' @param true A named list, setting the return value if \code{condition} is TRUE.
#'		Must have either \code{fixed_value} or \code{dynamic_value}.
#' @param false A named list, setting the return value if \code{condition} is FALSE.
#'		Must have either \code{fixed_value} or \code{dynamic_value}.
#' @param case A list of named lists, setting the return values if \code{condition == case}.
#'		Each vector represents one case; it must include a character string called \code{standard},
#'		which is the value to match against, as well as either \code{fixed_value} or
#'		\code{dynamic_value}.
#' @param default A named list, setting the default return value if \code{condition}
#'		doesn't match any case. Must have either \code{fixed_value} or
#'		\code{dynamic_value}.
#' @param id.name Character string, a unique ID for this property.
#'		If \code{"auto"}, IDs will be generated automatically from the condition ID.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' myCheckbox <- rk.XML.cbox("foo")
#' rk.XML.switch(myCheckbox,
#'   true=c(fixed_value="foo"),
#'   false=list(fixed_value="bar")
#' )
rk.XML.switch <- function(condition, modifier=NULL, true=NULL, false=NULL,
	case=NULL, default=NULL, id.name="auto"){

	condition.id <- check.ID(condition)
	if(is.XiMpLe.node(condition) && !is.null(modifier)){
		# validate modifier
		if(modif.validity(condition, modifier=modifier)){
			condition.id <- paste(condition.id, modifier, sep=".")
		} else {}
	} else {}

	if(identical(id.name, "auto")){
		attr.list <- list(condition=condition.id, id=auto.ids(condition.id, prefix=ID.prefix("switch")))
	} else if(!is.null(id.name)){
		attr.list <- list(condition=condition.id, id=id.name)
	} else {
		stop(simpleError("'id.name' must have a value!"))
	}

	check.required.attrs <- function(obj, req=c("fixed_value", "dynamic_value")){
			# some sanity check here
			if(!any(req %in% names(obj))){
				stop(simpleError("Check your attributes!"))
			} else {}
		}

	# does also check for the required default attributes
	check.dyn.value <- function(condCase){
			check.required.attrs(obj=condCase)
			if("dynamic_value" %in% names(condCase)){
				condCase[["dynamic_value"]] <- check.ID(condCase[["dynamic_value"]])
			} else {}
			return(condCase)
		}

	child.nodes <- list()
	if(!is.null(true) && !is.null(false)){
		true <- check.dyn.value(true)
		false <- check.dyn.value(false)
		child.nodes <- append(child.nodes, XMLNode("true", attrs=true))
		child.nodes <- append(child.nodes, XMLNode("false", attrs=false))
	} else if(!is.null(case)){
		for (thisNode in case){
			check.required.attrs(thisNode, req="standard")
			thisNode <- check.dyn.value(thisNode)
			child.nodes <- append(child.nodes, XMLNode("case", attrs=thisNode))
		}
		if(!is.null(default)){
			default <- check.dyn.value(default)
			child.nodes <- append(child.nodes, XMLNode("default", attrs=default))
		} else {}
	} else {
		stop(simpleError("Please either provide both 'true' and 'false', or 'case'!"))
	}

	node <- XMLNode("switch",
		attrs=attr.list,
		.children=child.nodes)

	return(node)

}
