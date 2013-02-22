#' Create JavaScript variables from optionsets
#' 
#' Scans an \code{<optionset>} node and returns the relevant variable or ID names of each
#' \code{<optioncolumn>} included.
#'
#' This function can be of assistance when dealing with \code{<optionset>} nodes, because what you
#' need in the JavaScript part of your plugin is the correct variable name of its columns.
#'
#' @param optionset An object of class \code{XiMpLe.node} containing a full \code{<optionset>}.
#' @param js Logical, if \code{TRUE} valid JavaScript varaible names will be returned, otherwise
#'		the XML ID names, respectively.
#' @param add.abbrev Logical, if \code{TRUE} the JavaScript variables will all have a prefix with an
#'		three letter abbreviation of the XML tag type to improve the readability of the code. But it's
#'		probably better to add this in the XML code in the first place.
#' @return A character vector. For each \code{<optioncolumn>} in the set, this vector will contain
#'		either its variable name or XML ID, depending on the \code{js} switch.
#' @seealso
#'		\code{\link[rkwarddev:rk.XML.optioncolumn]{rk.XML.optioncolumn}},
#'		\code{\link[rkwarddev:rk.XML.optiondisplay]{rk.XML.optiondisplay}},
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @export

# this is a quick hack which might become obsolete again, if i find a good way of limiting
# the JS variables to the column ID.

id.os <- function(optionset, js=TRUE, add.abbrev=FALSE){

	if(!is.XiMpLe.node(optionset) || !identical(XMLName(optionset), "optionset")){
		stop(simpleError("id.os() can only process <optionset> nodes in XiMpLe object form!"))
	} else {}

	# getting the relevant IDs out of optionsets is a little tricky
	# this function will probe for sets and return single tags
	single.tags <- check.optionset.tags(XML.obj=optionset, drop=c("comments","cdata", "declarations", "doctype"))

	JS.id <- get.IDs(single.tags=single.tags, relevant.tags="optioncolumn", add.abbrev=add.abbrev, tag.names=TRUE)

	if("id" %in% colnames(JS.id)){
		if(isTRUE(js)){
			JS.lines <- unlist(sapply(1:nrow(JS.id), function(this.id){
					return(rk.paste.JS(get.JS.vars(
						JS.var=JS.id[this.id,"abbrev"],
						XML.var=JS.id[this.id,"id"],
						tag.name=JS.id[this.id,"tag"],
						names.only=TRUE), level=1, indent.by=""))
				}, USE.NAMES=FALSE))
		} else {
			JS.lines <- JS.id[,"id"]
			names(JS.lines) <- NULL
		}
	} else {
		JS.lines <- NULL
	}

	return(JS.lines)
}
