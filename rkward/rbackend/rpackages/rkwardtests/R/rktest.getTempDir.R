#' Get the path to the recent temporary directory, if one exists.
#'
#' This function will return either the local path to the temporary directory where
#' all test results have been saved to, or FALSE if none exitsts.
#' 
#' @title Get path to the temporary directory
#' @usage rktest.getTempDir()
#' @aliases rktest.getTempDir
#' @return Either a character string, or FALSE.
#' @author Meik Michalke \email{meik.michalke@@uni-duesseldorf.de}
#' @keywords utilities
#' @seealso \code{\link[rkwardtests:rktest.makeplugintests]{rktest.makeplugintests}}
#' @export
#' @examples
#' rktest.getTempDir()

rktest.getTempDir <- function(){
  if(exists(".rktest.temp.dir", where=.rktest.tmp.storage)){
    temp.dir <- get(".rktest.temp.dir", pos=.rktest.tmp.storage)
    if(file_test("-d", temp.dir)) {
      return(temp.dir)
    }
    else {
      return(FALSE)
    }
  }
  else {
    return(FALSE)
  }
}
