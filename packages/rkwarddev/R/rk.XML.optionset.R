#' Create XML node "optionset" for RKWard plugins
#'
#' @note The \code{<optionset>} node was introduced with RKWard 0.6.1, please set the dependencies
#'		of your component/plugin accordingly.
#'
#' @param content A list of XiMpLe.nodes to be placed inside the <content> node of this <optionset>.
#' @param optioncolumn A list of \code{<optioncolumn>} XiMpLe.nodes.
#' @param min_rows Numeric (integer), if specified, the set will be marked invalid, unless it has
#'		at least this number of rows. Ignored if set to 0.
#' @param min_rows_if_any Numeric (integer), like min_rows, but will only be tested, if there is at
#'		least one row. Ignored if set to 0.
#' @param max_rows Numeric (integer), if specified, the set will be marked invalid, unless it has
#'		at most this number of rows. Ignored if set to 0.
#' @param keycolumn Character
#' @param logic A valid \code{<logic>} node.
#' @param optiondisplay Logical value, can be used to automatically add an \code{<optiondisplay>} node on top
#'		of the \code{<content>} section, if set to something other than \code{NULL}. Depending on whether it's
#'		\code{TRUE} or \code{FALSE}, its \code{index} argument will be set to \code{"true"} or
#'		\code{"false"}, respectively.
#' @param id.name Character string, a unique ID for this plugin element.
#'		If \code{"auto"}, an ID will be generated automatically from the <content> nodes.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\code{\link[rkwarddev:rk.XML.optioncolumn]{rk.XML.optioncolumn}},
#'		\code{\link[rkwarddev:rk.XML.optiondisplay]{rk.XML.optiondisplay}},
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
rk.XML.optionset <- function(content, optioncolumn, min_rows=0, min_rows_if_any=0, max_rows=0, keycolumn=NULL, logic=NULL, optiondisplay=NULL, id.name="auto"){

	if(identical(id.name, "auto")){
		# try autogenerating some id
		attr.list <- list(id=auto.ids(node.soup(content), prefix=ID.prefix("optionset"), chars=10))
	} else if(is.null(id.name)){
		attr.list <- list()
	} else {
		attr.list <- list(id=id.name)
	}

	if(min_rows > 0){
		attr.list[["min_rows"]] <- min_rows
	} else {}
	if(min_rows_if_any > 0){
		attr.list[["min_rows_if_any"]] <- min_rows_if_any
	} else {}
	if(max_rows > 0){
		attr.list[["max_rows"]] <- max_rows
	} else {}

	## TODO: do some checking -- and add support to supply XiMpLe nodes
	if(!is.null(keycolumn)){
		attr.list[["keycolumn"]] <- keycolumn
	} else {}

	if(!is.null(logic)){
		if(is.XiMpLe.node(logic) && identical(XMLName(logic), "logic")){
			valid.child("logic", children=XMLChildren(logic))
		} else {
			stop(simpleError("'logic' must be a <logic> node!"))
		}
		# checks go here
	} else {}

	# this list will carry all child nodes of the full set
	all.children <- list(logic)

	content <- child.list(content)
	optioncolumn <- child.list(optioncolumn)

	# auto-add optiondisplay
	if(!is.null(optiondisplay)){
		content <- append(content, rk.XML.optiondisplay(index=optiondisplay), after=0)
	} else {}

	content.node <- XMLNode("content",
		.children=content)

	# append content node
	all.children <- append(all.children, content.node)
	# append optioncolumns
	all.children <- append(all.children, optioncolumn)

	node <- XMLNode("optionset",
		attrs=attr.list,
		.children=all.children)

	return(node)
}
