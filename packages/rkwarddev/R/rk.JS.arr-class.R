#' @export

# this simple class is for JavaScript generation,
# produced by rk.JS.array()

setClass("rk.JS.arr",
  representation=representation(
    arr.name="character",
    opt.name="character",
    IDs="vector",
    variables="vector",
    funct="character",
    quote="logical",
    option="character"
  ),
  prototype(
    arr.name=character(),
    opt.name=character(),
    IDs=c(),
    variables=c(),
    funct="c",
    quote=FALSE,
    option=character()
  )
)
