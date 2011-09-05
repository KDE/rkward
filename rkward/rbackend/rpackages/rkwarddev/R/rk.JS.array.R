#' Create a simple JavaScript array
#'
#' If you need to combine multiple options (like values of several checkboxes) into one vector or list,
#' this function can help with that task. All relevant variables will become part of an array and
#' then joined into the desired argument type.
#'
#' @param option A character string, naming the option of, e.g., an R function which should be
#'		constructed from several variables.
#' @param variables A character vector, the names of the variables to combine to a vector or list.
#' @param list Logical, if \code{TRUE} the option will be constructed by \code{list()},
#'		otherwise by \code{c()}.
#' @param def.vars Logical, if \code{TRUE} the provided variables will also be defined.
#' @param var.prefix A character string. If \code{def.vars=TRUE}, this string will be used as a prefix
#'		for the JS variable names.
#' @param indent.by A character string defining how indentation should be done.
#' @return A character string.
#' @export
#' @examples
#' cat(rk.JS.array("my.option", variables=c("frst.var", "scnd.var")))

rk.JS.array <- function(option, variables=NULL, list=FALSE, def.vars=TRUE, var.prefix="chc", indent.by="\t"){
	arr.name <- camelCode(c("arr", option))
	opt.name <- camelCode(c("opt", option))

	if(isTRUE(def.vars)){
		JS.vars <- paste(unlist(sapply(variables, function(this.var){get.JS.vars(
							JS.var=this.var,
							XML.var=this.var,
							JS.prefix=var.prefix,
							indent.by=indent.by)
					})), collapse="")
	} else {
		JS.vars <- ""
	}

	JS.array <- paste(
		indent(2, by=indent.by), "var ", arr.name, " = new Array();\n",
		indent(2, by=indent.by), arr.name, ".push(",
		paste(unlist(sapply(variables, function(this.var){get.JS.vars(
							JS.var=this.var,
							JS.prefix=var.prefix,
							names.only=TRUE)
					})), collapse=", "), ");\n",
		indent(2, by=indent.by), arr.name, " = ", arr.name, ".filter(String);\n",
		indent(2, by=indent.by), "if(", arr.name, ".length > 0) {\n",
		indent(3, by=indent.by), "var ", opt.name, " = \", ", option,"=",
			ifelse(isTRUE(list), "list(", "c("),
			"\" + ", arr.name, ".join(\", \") + \")\";\n",
		indent(2, by=indent.by), "} else {\n",
		indent(3, by=indent.by), "var ", opt.name, " = \"\";\n",
		indent(2, by=indent.by), "}\n",
		sep="")

	results <- paste(JS.vars, JS.array, sep="\n")

	return(results)
}
