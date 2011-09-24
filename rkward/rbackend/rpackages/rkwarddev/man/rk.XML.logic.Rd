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
  This function will create a logic section with "convert"
  and "connect" nodes.
}
\examples{
test.convert <- rk.XML.convert(c(string="foo"), mode=c(notequals="bar"), id.name="lgc_foobar")
test.connect <- rk.XML.connect(governor="lgc_foobar", client="frame_bar")
test.logic <- rk.XML.logic(nodes=list(test.convert, test.connect))
cat(pasteXMLNode(test.logic, shine=1))
}

