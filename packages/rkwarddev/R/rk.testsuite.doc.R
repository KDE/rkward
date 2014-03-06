#' Create testsuite outline to test an RKWard plugin
#'
#' @param name A character string, name of the plugin/dialog.
#' @return A character string.
#' @seealso \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @export

rk.testsuite.doc <- function(name=NULL){
  suite.text <- paste0("## definition of the test suite
  suite <- new(\"RKTestSuite\",
    id=\"", name ,"\",
    ## needed packages
    libraries = c(\"", name ,"\"),
    ## initCalls are run *before* any tests. Use this to set up the environment
    initCalls = list(
      function(){
        ## e.g. load needed packages
        # require(\"package\")

        ## or prepare needed data objects
        # data(\"sampledata\")

        ## or create needed objects
        # object <- NULL
        # assign(\"object.name\", object, envir=globalenv())

      },
      function(){
        ## if some tests depend on results of earlier tests,
        ## you can store those in a list in .GlobalEnv
        # earlier.results <<- list()
      }
    ),
    ## the tests
    tests = list(
        ## define the actual tests here
        # new(\"RKTest\", id=\"\", call=function(){
        #  rk.call.plugin(\"rkward::...\", ..., submit.mode=\"submit\")
        ## to store these results:
        #  earlier.results$this.result1 <<- this.result
        # }),
        # new(\"RKTest\", id=\"\", call=function(){
        #  rk.call.plugin(\"rkward::...\", ..., submit.mode=\"submit\")
        ## to store these results:
        #  earlier.results$this.result2 <<- this.result
        # })
    ),
    ## postCalls are like initCalls, but run after all tests to clean up.
    postCalls = list(
      function(){
        ## e.g. remove created objects
        # rm(list=c(\"earlier.results\"), envir=globalenv())
      }
    )
  )")

  return(suite.text)
}
