\name{rk.XML.tabbook}
\alias{rk.XML.tabbook}
\title{Create XML node "tabbook" for RKWard plugins}
\usage{
  rk.XML.tabbook(label = NULL, tab.labels, children =
  list(), id.name = "auto")
}
\arguments{
  \item{label}{Character string, a text label for this
  plugin element. Must be set if \code{id.name="auto"}.}

  \item{tab.labels}{Character vector, where each string
  defines the name of one tab. The number of
  \code{tab.labels} also defines the number of tabs.}

  \item{children}{An optional list with objects of class
  \code{XiMpLe.node} (or a list of these objects). You must
  provide one element for each tab. Use \code{NULL} for
  tabs without predefined children.}

  \item{id.name}{Character vector, unique IDs for the
  tabbook (first entry) and all tabs. If \code{"auto"}, IDs
  will be generated automatically from the labels. If
  \code{NULL}, no IDs will be given.}
}
\value{
  An object of class \code{XiMpLe.node}.
}
\description{
  Create XML node "tabbook" for RKWard plugins
}
\examples{
test.checkboxes <- rk.XML.row(rk.XML.col(
list(
rk.XML.cbox(label="foo", val="foo1", chk=TRUE),
rk.XML.cbox(label="bar", val="bar2"))))
test.dropdown <- rk.XML.dropdown("mydrop",
opts=list("First Option"=c(val="val1"),
"Second Option"=c(val="val2", chk=TRUE)))
# combine the above into a tabbook
test.tabbook <- rk.XML.tabbook("My Tabbook", tab.labels=c("First Tab",
"Second Tab"), children=list(test.checkboxes, test.dropdown))
cat(pasteXMLNode(test.tabbook))
}

