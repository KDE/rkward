\name{rk.XML.dialog}
\alias{rk.XML.dialog}
\title{Create XML dialog section for RKWard plugins}
\usage{
  rk.XML.dialog(nodes)
}
\arguments{
  \item{nodes}{A (list of) objects of class
  \code{XiMpLe.node}.}
}
\value{
  A list of objects of class \code{XiMpLe.node}.
}
\description{
  This function will create a dialog section with optional
  child nodes "browser", "checkbox", "column", "copy",
  "dropdown", "formula", "frame", "input", "radio", "row",
  "saveobject", "spinbox", "stretch", "tabbook", "text",
  "varselector" and "varslot".
}
\examples{
# define an input field and two checkboxes
test.input <- rk.XML.input("Type some text")
test.cbox1 <- rk.XML.cbox(label="Want to type?", val="true")
test.cbox2 <- rk.XML.cbox(label="Are you shure?", val="true")
test.dialog <- rk.XML.dialog(rk.XML.col(list(test.input, test.cbox1, test.cbox2)))
cat(pasteXMLNode(test.dialog, shine=1))
}
\seealso{
  \code{\link[rkwarddev:rk.XML.plugin]{rk.XML.plugin}},
  \code{\link[rkwarddev:rk.plugin.skeleton]{rk.plugin.skeleton}}
}

