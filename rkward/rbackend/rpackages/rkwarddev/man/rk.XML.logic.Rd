\name{rk.XML.logic}
\alias{rk.XML.logic}
\title{Create XML logic section for RKWard plugins}
\usage{
  rk.XML.logic(nodes)
}
\arguments{
  \item{nodes}{A list of objects of class
  \code{XiMpLe.node}.}
}
\value{
  A list of objects of class \code{XiMpLe.node}.
}
\description{
  This function will create a logic section with "convert",
  "connect" ,"external" and "set" nodes.
}
\examples{
test.convert <- rk.XML.convert(c(string="foo"), mode=c(notequals="bar"), id.name="lgc_foobar")
test.connect <- rk.XML.connect(governor="lgc_foobar", client="frame_bar")
test.external <- rk.XML.external(id="ext_property", default="none")
test.logic <- rk.XML.logic(nodes=list(test.convert, test.connect, test.external))
cat(pasteXMLNode(test.logic, shine=1))
}

