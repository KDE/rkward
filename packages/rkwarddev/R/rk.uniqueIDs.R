#' Check plugin dialogs for duplicate IDs
#'
#' A plugin must not have duplicated IDs to work properly. This function
#' cannot automatically correct duplicates, but it will warn you about
#' it, so you can correct your plugin script manually
#'
#' @param obj An XML object of class XiMpLe.node or XiMpLe.doc.
#' @param bool Logical, if \code{TRUE} this function will return a logical value.
#' @param warning Logical, if \code{TRUE} will throw a warning if duplicates were found,
#'		listing the findings.
#' @return A vector with duplicate IDs, if any were found.
#'		If \code{bool=TRUE} returns a logical value.

rk.uniqueIDs <- function(obj, bool=FALSE, warning=TRUE){
	allIDs <- XMLScanDeep(obj, find="id", search="attributes")
	duplicateIDs <- duplicated(allIDs)
	if(any(duplicateIDs)){
		# ok, let's get 'em
		invalidIDs <- unique(allIDs[duplicateIDs])
		result <- allIDs[allIDs %in% invalidIDs]
		if(isTRUE(warning)){
			warning(paste0("Duplicate IDs wer found:\n  ", paste0(names(result), ": ", result, collapse="\n  ")), call.=FALSE)
		} else {}
		if(isTRUE(bool)){
			return(FALSE)
		} else {
			return(result)
		}
	} else {
		if(isTRUE(bool)){
			return(TRUE)
		} else {
			return(invisible(NULL))
		}
	}
}

