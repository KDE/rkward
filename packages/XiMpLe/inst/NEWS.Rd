\name{NEWS}
\title{News for Package 'XiMpLe'}
\encoding{UTF-8}
\section{Changes in XiMpLe version 0.03-12 (2012-03-16)}{
  \itemize{
    \item \code{node()} now also works with single XiMpLe.node objects (not only full trees)
    \item added pasteXML methods to paste objects of classes XiMpLe.node and XiMpLe.doc in a more
      general manner. the former functions \code{pasteXMLNode()} and \code{pasteXMLTree()} are replaced by mere
      wrapper frunctions to \code{pasteXML()} and should not be used any longer.
    \item fixed dropped text value if node also had child nodes in \code{pasteXML()}
  }
}
\section{Changes in XiMpLe version 0.03-11 (2012-03-14)}{
  \itemize{
    \item added functions \code{XMLNode()} and \code{XMLTree()} as constructor functions for XML nodes and trees.
    \item added opton \code{"object"} to \code{parseXMLTree()}, to be able to parse XML trees not only from files, but also character vectors
    \item output of internal function \code{XML.single.tags()} has no colname any more
    \item text values of nodes are no longer followed by an empty newline in \code{pasteXMLNode()}
  }
}
\section{Changes in XiMpLe version 0.03-10 (2012-03-11)}{
  \itemize{
    \item fixed bug in parsing of attributes, their names are now all quoted by default o not run into issues
      if they contain R-sensitive characters (like "http-equiv")
  }
}
\section{Changes in XiMpLe version 0.03-9 (2012-03-10)}{
  \itemize{
    \item re-formatted the ChangeLog so roxyPackage can translate it into a NEWS.Rd file
  }
}
\section{Changes in XiMpLe version 0.03-8 (2011-11-17)}{
  \itemize{
    \item rewrote large parts of internal function \code{XML.single.tags()} for more efficiency, allthough it's still quite
      lethargic when handling huge XML trees
  }
}
\section{Changes in XiMpLe version 0.03-7 (2011-10-23)}{
  \itemize{
    \item added "&" to the special characters for \code{"tidy"} (only applies if theres space before and after)
    \item \code{"tidy"} now also indents text in comments and CDATA if it includes the newline character
  }
}
\section{Changes in XiMpLe version 0.03-6 (2011-10-09)}{
  \itemize{
    \item added \code{"tidy"} option to the paste functions, which is TRUE by default since "<" and ">" are
      invalid in attributes and text anyway
  }
}
\section{Changes in XiMpLe version 0.03-5 (2011-10-07)}{
  \itemize{
    \item fixed tree parsing bug (in rare cases, comment and CDATA entries caused parsing to choke on
      empty lines)
  }
}
\section{Changes in XiMpLe version 0.03-4 (2011-10-01)}{
  \itemize{
    \item added show methods for objects of class XiMpLe.doc and XiMpLe.node
  }
}
\section{Changes in XiMpLe version 0.03-3 (2011-09-27)}{
  \itemize{
    \item changed default 'shine' value of \code{pasteXMLNode()} and \code{pasteXMLTree()} from 2 to 1
    \item \code{pasteXMLNode()} and \code{pasteXMLTree()} now try to call each other if fed with the wrong class object,
      but a warning is given
    \item changed handling of 'value' in \code{pasteXMLNode()}, now reflects 'shine' setting/indentation
  }
}
\section{Changes in XiMpLe version 0.03-2 (2011-09-17)}{
  \itemize{
    \item added docs for classed XiMpLe.doc and XiMpLe.node
  }
}
\section{Changes in XiMpLe version 0.03-1 (2011-09-11)}{
  \itemize{
    \item fixed outstanding issue with complex value structures
    \item changed "Author@R" to "Authors@R" in DESCRIPTION (for R >= 2.14)
  }
}
\section{Changes in XiMpLe version 0.02-2 (2011-09-01)}{
  \itemize{
    \item fixed missing \code{"methods"} dependecy
    \item adjusted docs a little
  }
}
\section{Changes in XiMpLe version 0.02-1 (2011-08-30)}{
  \itemize{
    \item added option \code{"indent.by"} to the paste functions, in case one prefers space over tab.
    \item moved all rk.* functions out of this package and into its own (rkwardplugdev)
  }
}
\section{Changes in XiMpLe version 0.01-9 (2011-08-28)}{
  \itemize{
    \item tiny improvements to XML formatting
    \item added functions \code{rk.XML.tabbook()}, \code{rk.XML.dropdown()}, \code{rk.XML.plugin()}
      rk.\code{XML.pluginmap()} and \code{rk.plugin.skeleton()} [to be moved again]
  }
}
\section{Changes in XiMpLe version 0.01-8 (2011-08-26)}{
  \itemize{
    \item improved handling of comments a lot
  }
}
\section{Changes in XiMpLe version 0.01-7 (2011-08-24)}{
  \itemize{
    \item fixed typo bug in \code{XML.single.tags()}
    \item improved support for comments and CDATA
    \item improved XML formatting
  }
}
\section{Changes in XiMpLe version 0.01-6 (2011-08-23)}{
  \itemize{
    \item added functions \code{pasteXMLTree()} and \code{pasteXMLNode()}
    \item added validity checks for classes XiMpLe.doc and XiMpLe.node
    \item added function \code{rk.XML.about()} [which will probably be moved out of the package]
  }
}
\section{Changes in XiMpLe version 0.01-5 (2011-08-22)}{
  \itemize{
    \item writing new node values with node<-() partly works
  }
}
\section{Changes in XiMpLe version 0.01-4 (2011-08-21)}{
  \itemize{
    \item rewrote and renamed \code{getNode()} to \code{node()}
  }
}
\section{Changes in XiMpLe version 0.01-3 (2011-08-18)}{
  \itemize{
    \item checks XML and DTD declaration now
    \item added \code{getNode()} method
  }
}
\section{Changes in XiMpLe version 0.01-2 (2011-08-15)}{
  \itemize{
    \item iterating through an XML tree actually works now
  }
}
\section{Changes in XiMpLe version 0.01-1 (2011-08-14)}{
  \itemize{
    \item initial release via reaktanz.de
  }
}
