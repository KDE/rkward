#' Create JavaScript variables from plugin XML
#'
#' @param pXML Either an object of class \code{XiMpLe.doc} or \code{XiMpLe.node}, or path to a plugin XML file.
#' @param js Logical, if \code{TRUE} usable JavaScript code will be returned, otherwise a character
#'		vector with only the relevant ID names.
#' @param add.abbrev Logical, if \code{TRUE} the JavaScript variables will all have a prefix with an
#'		three letter abbreviation of the XML tag type to improve the readability of the code. But it's
#'		probably better to add this in the XML code in the first place.
#' @param indent.by Character string used to indent each entry if \code{js=TRUE}.
#' @return A character vector.
#' @seealso \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @export

rk.JS.scan <- function(pXML, js=TRUE, add.abbrev=FALSE, indent.by="\t"){

	JS.relevant.tags <- c("radio", "varslot", "browser", "dropdown",
		"checkbox", "saveobject", "input", "spinbox")

	single.tags <- get.single.tags(XML.obj=pXML, drop=c("comments","cdata", "declarations", "doctype"))

	JS.id <- get.IDs(single.tags=single.tags, relevant.tags=JS.relevant.tags, add.abbrev=add.abbrev)

	if("id" %in% colnames(JS.id)){
		if(isTRUE(js)){
			# now
			#   <tag id="my.id" ...>
			# will become
			#   var my.id = getValue("my.id");
			JS.lines <- paste(unlist(sapply(1:nrow(JS.id), function(this.id){
					return(get.JS.vars(
						JS.var=JS.id[this.id,"abbrev"],
						XML.var=JS.id[this.id,"id"],
						indent.by=indent.by))
				})), collapse="")
		} else {
			JS.lines <- JS.id[,"id"]
			names(JS.lines) <- NULL
		}
	} else {
		JS.lines <- NULL
	}

	return(JS.lines)
}
