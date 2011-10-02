\name{echo}
\alias{echo}
\title{Generate JavaScript echo command call}
\usage{
  echo(..., newline = "")
}
\arguments{
  \item{...}{One or several character strings and/or
  \code{XiMpLe.node} objects with plugin nodes, simply
  separated by comma.}

  \item{newline}{Character string, can be set to e.g.
  \code{"\n"} to force a newline after the call.}
}
\value{
  A character string.
}
\description{
  This function will take several elements, either
  character strings or objects of class \code{XiMpLe.node}
  which hold an XML node of some plugin GUI definition, and
  generate a ready-to-run JavaScript \code{echo();} call
  from it.
}

