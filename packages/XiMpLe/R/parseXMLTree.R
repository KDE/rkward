#' Read an XML file into an R object
#'
#' @param file Character string, valid path to the XML file which should be parsed.
#' @param drop Character vector with the possible values \code{"comments"}, \code{"cdata"}
#'		\code{"declarations"} and \code{"doctype"}, defining element classes to be dropped
#'		from the resulting object.
#' @param object Logical, if \code{TRUE}, \code{file} will not be treated as a path name but as a
#'		character vector to be parsed as XML directly.
#' @return An object of class \code{XiMpLe.doc} with four slots:
#'		\describe{
#'			\item{\code{file}:}{Full path to the parsed file, or \code{"object"} if \code{object=TRUE}.}
#'			\item{\code{xml}:}{XML declaration, if found.}
#'			\item{\code{dtd}:}{Doctype definition, if found.}
#'			\item{\code{children}:}{A list of objects of class \code{XiMpLe.node}, with the elements
#'				\code{"name"} (the node name), \code{"attributes"} (list of attributes, if found),
#'				\code{"children"} (list of \code{XiMpLe.node} object, if found) and \code{"value"}
#'				(text value between a pair of start/end tags, if found).}
#'		}
#' @export
#' @examples
#' \dontrun{
#' sample.XML.object <- parseXMLTree("~/data/sample_file.xml")
#' }

parseXMLTree <- function(file, drop=NULL, object=FALSE){
	if(isTRUE(object)){
		xml.raw <- paste(file, collapse="\n")
		filePath <- "object"
	} else if(inherits(file, "connection")){
		xml.raw <- paste(readLines(file), collapse="\n")
		# is there a way to get the friggin' "description" out of a connection object?!
		filePath <- "connection"
	} else {
		xml.raw <- paste(readLines(file), collapse="\n")
		# try to detect if 'file' is like a weblink, not a regular file
		if(grepl("^[[:alpha:]]+://", file, ignore.case=TRUE)){
			filePath <- file
		} else {
			filePath <- normalizePath(file)
		}
	}

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
		file=filePath,
		xml=XML.decl,
		dtd=XML.doct,
		children=children)
	
	return(results)
}
