# package description files
# this internal object can be used by the package roxyPackage to
# automatically create/update DESCRIPTION and CITATION files
pckg.dscrptn <- data.frame(
		Package="XiMpLe",
		Type="Package",
		Title="A simple XML tree parser and generator",
		Author="m.eik michalke <meik.michalke@hhu.de>",
		AuthorsR="c(person(given=\"Meik\", family=\"Michalke\", email=\"meik.michalke@hhu.de\",
			role=c(\"aut\", \"cre\")))",
		Maintainer="m.eik michalke <meik.michalke@hhu.de>",
		Depends="R (>= 2.9.0),methods",
		Suggests="testthat",
		Enhances="rkward",
		Description="This package provides a simple XML tree parser/generator. It includes functions to read XML files into R objects,
						get information out of and into nodes, and write R objects back to XML code.
						It's not as powerful as the XML package and doesn't aim to be, but for simple XML handling
						it could be useful. It was originally programmed for RKWard.",
		License="GPL (>= 3)",
		Encoding="UTF-8",
		LazyLoad="yes",
		URL="http://reaktanz.de/?c=hacking&s=XiMpLe",
		stringsAsFactors=FALSE)
