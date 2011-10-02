#' Read an XML file into an R object
#'
#' @param file Character string, valid path to the XML file which should be parsed.
#' @param drop Character vector with the possible values \code{"comments"}, \code{"cdata"}
#'		\code{"declarations"} and \code{"doctype"}, defining element classes to be dropped
#'		from the resulting object.
#' @return An object of class \code{XiMpLe.doc} with four slots:
#'		\describe{
#'			\item{\code{file}:}{Full path to the parsed file.}
#'			\item{\code{xml}:}{XML declaration, if found.}
#'			\item{\code{dtd}:}{Doctype definition, if found.}
#'			\item{\code{children}:}{A list of objects of class \code{XiMpLe.node}, with the elements
#'				\code{"name"} (the node name), \code{"attributes"} (list of attributes, if found),
#'				\code{"children"} (list of \code{XiMpLe.node} object, if found) and \code{"value"}
#'				(text value between a pair of start/end tags, if found).}
#'		}
#' @export

parseXMLTree <- function(file, drop=NULL){
	xml.raw <- paste(readLines(file), collapse=" ")

	single.tags <- XML.single.tags(xml.raw, drop=drop)

	# check for XML declaration and doctype first
	if(XML.declaration(single.tags[1])){
		XML.decl <- parseXMLAttr(single.tags[1])
		single.tags <- single.tags[-1]
	} else {
		XML.decl <- list(version="", encoding="", standalone="")
	}
	if(any(XML.doctype(single.tags[1]))){
		XML.doct <- parseXMLAttr(single.tags[1])
		single.tags <- single.tags[-1]
	} else {
		XML.doct <- list(doctype="", id="", decl="", refer="")
	}
	# try to iterate through the single tags
	children <- XML.nodes(single.tags)[["children"]]
	
	results <- new("XiMpLe.doc",
		file=normalizePath(file),
		xml=XML.decl,
		dtd=XML.doct,
		children=children)
	
	return(results)
}
