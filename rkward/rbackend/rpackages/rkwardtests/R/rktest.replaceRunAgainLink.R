#' Replace "Run again" link in RKWard with code
#'
#' You can use this to temporarily replace .rk.rerun.plugin.link (see example below).
#' This way, after running a plugin, you are shown the call needed to run this
#' plugin with those settings, instead of the link.
#'
#' This code can be used in a plugin test suite.
#' 
#' @title Replace "Run again" link in RKWard
#' @usage rktest.replaceRunAgainLink(restore=FALSE)
#' @aliases .rk.rerun.plugin.link.replacement
#' @param restore Logical: If TRUE, restore the original behaviour.
#' @return Replaces the "Run again" link in RKWard with the code that would have been called, or vice versa.
#' @docType function
#' @author Thomas Friedrichsmeier \email{thomas.friedrichsmeier@@ruhr-uni-bochum.de}, Meik Michalke \email{meik.michalke@@uni-duesseldorf.de}
#' @keywords utilities
#' @seealso \code{\link[rkwardtests:RKTestSuite]{RKTestSuite-class}}, \code{\link[rkwardtests:rktest.makeplugintests]{rktest.makeplugintests}}
#' @export
#' @rdname rktest.replaceRunAgainLink
#' @examples
#' rktest.replaceRunAgainLink()
#' }

rktest.replaceRunAgainLink <- function(restore=FALSE){
  if(!restore){
    # check if there's already a backup
    if(!exists(".rktest.replaceRunAgainLink.restore", where=globalenv())){
      replace <- get(".rk.rerun.plugin.link", pos=globalenv())
      assign(".rktest.replaceRunAgainLink.restore", replace, envir=globalenv())
      assign(".rk.rerun.plugin.link", .rk.rerun.plugin.link.replacement, envir=globalenv())
    }
    else {
      stop(simpleWarning("Found a backup to restore -- have you already replaced the link?"))
    }
  }
  else {
    if(exists(".rktest.replaceRunAgainLink.restore", where=globalenv())){
      restore <- get(".rktest.replaceRunAgainLink.restore", pos=globalenv())
      assign(".rk.rerun.plugin.link", restore, envir=globalenv())
      rm(".rktest.replaceRunAgainLink.restore", pos=globalenv())
    }
    else {
      stop(simpleWarning("No backup to restore found!"))
    }
  }
}
