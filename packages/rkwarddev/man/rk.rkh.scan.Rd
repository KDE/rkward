\name{rk.rkh.scan}
\alias{rk.rkh.scan}
\title{Create RKWard help nodes from plugin XML}
\usage{
  rk.rkh.scan(pXML, help = TRUE)
}
\arguments{
  \item{pXML}{Either an object of class \code{XiMpLe.doc}
  or \code{XiMpLe.node}, or path to a plugin XML file.}

  \item{help}{Logical, if \code{TRUE} a list of XiMpLe.node
  objects will be returned, otherwise a character vector
  with only the relevant ID names.}
}
\value{
  A character vector or a list of XiMpLe.node objects.
}
\description{
  Create RKWard help nodes from plugin XML
}

