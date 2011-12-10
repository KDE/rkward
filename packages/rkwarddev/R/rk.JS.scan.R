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
					return(rk.paste.JS(get.JS.vars(
						JS.var=JS.id[this.id,"abbrev"],
						XML.var=JS.id[this.id,"id"]),
						level=2, indent.by=indent.by))
				})), collapse="\n")
		} else {
			JS.lines <- JS.id[,"id"]
			names(JS.lines) <- NULL
		}
	} else {
		JS.lines <- NULL
	}

	# special tags: must be chackable and get "checked" property
	JS.special.tags <- c("frame")
	JS.special.id <- get.IDs(single.tags=single.tags, relevant.tags=JS.special.tags, add.abbrev=add.abbrev, only.checkable=TRUE)
	if("id" %in% colnames(JS.special.id)){
		if(isTRUE(js)){
			JS.lines <- paste(JS.lines, "\n", paste(unlist(sapply(1:nrow(JS.special.id), function(this.id){
					return(rk.paste.JS(get.JS.vars(
						JS.var=JS.special.id[this.id,"abbrev"],
						XML.var=JS.special.id[this.id,"id"],
						modifiers="checked"),
						level=2, indent.by=indent.by))
				})), collapse="\n"), sep="")
		} else {
			JS.lines <- c(JS.lines, JS.special.id[,"id"])
			names(JS.lines) <- NULL
		}
	} else {}

	return(JS.lines)
}
