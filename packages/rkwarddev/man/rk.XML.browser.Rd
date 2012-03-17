\name{rk.XML.browser}
\alias{rk.XML.browser}
\title{Create XML node "browser" for RKWard plugins}
\usage{
  rk.XML.browser(label, type = "file", initial = NULL,
    urls = FALSE, filter = NULL, required = TRUE,
    id.name = "auto")
}
\arguments{
  \item{label}{Character string, a text label for this
  plugin element.}

  \item{type}{Character string, valid values are "dir",
  "file" and "savefile" (i.e., an non-existing file).}

  \item{initial}{Character string, if not \code{NULL} will
  be used as the initial value of the browser.}

  \item{urls}{Logical, whether non-local URLs are permitted
  or not.}

  \item{filter}{Character vector, file type filter, e.g.
  \code{filter=c("*.txt", "*.csv")} for .txt and .csv
  files.  Try not to induce limits unless absolutely
  needed, though.}

  \item{required}{Logical, whether an entry is mandatory or
  not.}

  \item{id.name}{Character string, a unique ID for this
  plugin element.  If \code{"auto"} and a label was
  provided, an ID will be generated automatically from the
  label.}
}
\value{
  An object of class \code{XiMpLe.node}.
}
\description{
  Create XML node "browser" for RKWard plugins
}
\examples{
test.browser <- rk.XML.browser("Browse here:")
cat(pasteXML(test.browser))
}
\seealso{
  \href{help:rkwardplugins}{Introduction to Writing Plugins
  for RKWard}
}

