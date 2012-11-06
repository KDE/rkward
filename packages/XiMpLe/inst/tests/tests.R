context("XML generation")

test_that("generate empty XML node", {
	sampleXMLStandard <- dget("sample_XML_node_empty_dput.txt")
	expect_that(
		XMLNode("empty"),
		equals(sampleXMLStandard)
	)
})

test_that("generate closed XML node", {
	sampleXMLStandard <- dget("sample_XML_node_closed_dput.txt")
	expect_that(
		XMLNode("empty", ""),
		equals(sampleXMLStandard)
	)
})

test_that("generate closed XML node with attributes", {
	# re-create object sampleXMLnode.attrs
	load("sample_XML_node_attrs.RData")
	expect_that(
		XMLNode("empty", "test", attrs=list(foo="bar")),
		equals(sampleXMLnode.attrs)
	)
})

test_that("generate nested XML tag tree", {
	# re-create object sampleXMLTree
	load("sample_XML_tree.RData")

	sampleXMLnode.empty <- XMLNode("empty")
	sampleXMLnode.closed <- XMLNode("empty", "")
	sampleXMLnode.attrs <- XMLNode("empty", "test", attrs=list(foo="bar"))
	sampleXMLTree.test <- XMLTree(
		XMLNode("tree",
			sampleXMLnode.empty,
			sampleXMLnode.closed,
			sampleXMLnode.attrs
		)
	)

	expect_that(
		sampleXMLTree.test,
		equals(sampleXMLTree)
	)
})


context("XML parsing")

test_that("parse XML file", {
	# re-create object sampleXMLparsed
	load("sample_RSS_parsed.RData")

	sampleXMLFile <- normalizePath("koRpus_RSS_sample.xml")
	XMLtoParse <- file(sampleXMLFile, encoding="UTF-8")
	sampleXMLparsed.test <- parseXMLTree(XMLtoParse)
	close(XMLtoParse)

 	expect_that(
		sampleXMLparsed.test,
 		equals(sampleXMLparsed))
})


context("extracting nodes")

test_that("extract node from parsed XML tree", {
	# re-create object sampleXMLparsed
	load("sample_RSS_parsed.RData")
	# re-create object sampleXMLnode.extracted
	load("sample_XML_node_extracted.RData")

	sampleXMLnode.test <- node(sampleXMLparsed, node=list("rss","channel","atom:link"))

 	expect_that(
		sampleXMLnode.test,
 		equals(sampleXMLnode.extracted))
})


context("changing node values")

test_that("change attribute values in XML node", {
	# re-create object sampleXMLparsed
	load("sample_RSS_parsed.RData")
	# re-create object sampleXMLnode.extracted
	load("sample_XML_tree_changed.RData")

	# replace URL
	node(sampleXMLparsed,
		node=list("rss","channel","atom:link"),
		what="attributes", element="href") <- "http://example.com"

	# remove "rel" attribute
	node(sampleXMLparsed,
		node=list("rss","channel","atom:link"),
		what="attributes", element="rel") <- NULL

 	expect_that(
		sampleXMLparsed,
 		equals(sampleXMLparsed.changed))
})

test_that("change nested text value in XML node", {
	# re-create object sampleXMLparsed
	load("sample_RSS_parsed.RData")
	# re-create object sampleXMLnode.extracted
	load("sample_XML_tree_changed_value.RData")

	# change text
	node(sampleXMLparsed,
		node=list("rss","channel","item","title"),
		what="value",
		cond.value="Changes in koRpus version 0.04-30") <- "this value was changed!"

 	expect_that(
		sampleXMLparsed,
 		equals(sampleXMLparsed.changed.value))
})
