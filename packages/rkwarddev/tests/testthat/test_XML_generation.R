# testing basic tokenizing and POS tagging

# all test standards are stored in a list object called "XML_test_standards"
# it was saved to the file "XML_test_standards.RData"

context("XML")

test_that("rk.XML.about", {
    thisNode <- rk.XML.about(
        name="Square the circle",
        author=c(
            person(given="E.A.", family="Dölle",
                email="doelle@eternalwondermaths.example.org", role="aut"),
            person(given="A.", family="Assistant",
                email="alterego@eternalwondermaths.example.org", role=c("cre","ctb"))
        ),
        about=list(
        desc="Squares the circle using Heisenberg compensation.",
        version="0.1-3",
        date=as.Date("2014-12-12"),
        url="http://eternalwondermaths.example.org/23/stc.html",
        license="GPL",
        category="Geometry")
    )
    load("XML_test_standards.RData")
    expect_that(
        thisNode, equals(XML_test_standards[["about"]])
    )
})

test_that("rk.XML.attribute", {
    thisNode <- rk.XML.attribute(
        id="testID",
        label="an attribute label",
        value="checked"
    )
    load("XML_test_standards.RData")
    expect_that(
        thisNode, equals(XML_test_standards[["attribute"]])
    )
})

test_that("rk.XML.browser", {
    thisNode <- rk.XML.browser(
        label="a browser label",
        type="dir",
        initial="/tmp/initial",
        filter=c("*initial", "*csv"),
        required=TRUE,
        i18n=list(context="context info here")
    )
    load("XML_test_standards.RData")
    expect_that(
        thisNode, equals(XML_test_standards[["browser"]])
    )
})

test_that("rk.XML.cbox", {
    thisNode <- rk.XML.cbox(
        label="a checbox label",
        value="checked",
        chk=TRUE,
        un.value="unchecked",
        i18n=list(context="context info here")
    )
    load("XML_test_standards.RData")
    expect_that(
        thisNode, equals(XML_test_standards[["checkbox"]])
    )
})

test_that("rk.XML.code", {
    thisNode <- rk.XML.code(
        file="some_file.js"
    )
    load("XML_test_standards.RData")
    expect_that(
        thisNode, equals(XML_test_standards[["code"]])
    )
})

test_that("rk.XML.col", {
    thisNode <- rk.XML.col(
        rk.XML.cbox(label="a column label")
    )
    load("XML_test_standards.RData")
    expect_that(
        thisNode, equals(XML_test_standards[["col"]])
    )
})

test_that("rk.XML.component", {
    thisNode <- rk.XML.component(
        label="a component label",
        file="plugins/MyGUIdialog.xml",
        i18n=list(context="context info here")
    )
    load("XML_test_standards.RData")
    expect_that(
        thisNode, equals(XML_test_standards[["component"]])
    )
})

test_that("rk.XML.components", {
    thisNode <- rk.XML.components(
        rk.XML.component(
            label="a components label",
            file="plugins/MyOtherGUIdialog.xml"
        )
    )
    load("XML_test_standards.RData")
    expect_that(
        thisNode, equals(XML_test_standards[["components"]])
    )
})

test_that("rk.XML.connect", {
    thisNode <- rk.XML.connect(
        governor="myGovernor",
        client="myCLient",
        get="visible",
        set="string",
        not=TRUE,
        reconcile=TRUE
    )
    load("XML_test_standards.RData")
    expect_that(
        thisNode, equals(XML_test_standards[["connect"]])
    )
})

test_that("rk.XML.context", {
    thisNode <- rk.XML.context(
        rk.XML.menu("Analysis", 
            rk.XML.entry(
                rk.XML.component(
                    label="a context label",
                    file="plugins/MyOtherGUIdialog.xml"
                )
            )
        )
    )
    load("XML_test_standards.RData")
    expect_that(
        thisNode, equals(XML_test_standards[["context"]])
    )
})

test_that("rk.XML.convert", {
    thisNode <- rk.XML.convert(
        sources=list(text=rk.XML.input("a convert label")),
        mode=c(equals="myValue"),
        required=TRUE
    )
    load("XML_test_standards.RData")
    expect_that(
        thisNode, equals(XML_test_standards[["convert"]])
    )
})

test_that("rk.XML.copy", {
    thisNode <- rk.XML.copy(
        id="aCopyID",
        as="page"
    )
    load("XML_test_standards.RData")
    expect_that(
        thisNode, equals(XML_test_standards[["copy"]])
    )
})

test_that("rk.XML.dependencies", {
    thisNode <- rk.XML.dependencies(
        dependencies=list(
            rkward.min="0.6.3",
            rkward.max="0.6.5",
            R.min="3.1",
            R.max="3.2"
        ),
        package=list(
            c(name="heisenberg", min="0.11-2", max="0.14-1", repository="http://hsb.example.org"),
            c(name="DreamsOfPi", min="0.2", max="3.1", repository="http://dop.example.org")
        ),
        pluginmap=list(
            c(name="heisenberg.pluginmap", url="http://eternalwondermaths.example.org/hsb")
        )
    )
    load("XML_test_standards.RData")
    expect_that(
        thisNode, equals(XML_test_standards[["dependencies"]])
    )
})

test_that("rk.XML.dependency_check", {
    thisNode <- rk.XML.dependency_check(
        id.name="dependency",
        dependencies=list(
            rkward.min="0.6.3",
            rkward.max="0.6.5",
            R.min="3.1",
            R.max="3.2"),
        package=list(
            c(name="heisenberg", min="0.11-2", max="0.14-1",
                repository="http://hsb.example.org"),
            c(name="DreamsOfPi", min="0.2", max="3.1", repository="http://dop.example.org")),
        pluginmap=list(
            c(name="heisenberg.pluginmap", url="http://eternalwondermaths.example.org/hsb"))
    )
    load("XML_test_standards.RData")
    expect_that(
        thisNode, equals(XML_test_standards[["dependency_check"]])
    )
})

test_that("rk.XML.dialog", {
    thisNode <- rk.XML.dialog(
        rk.XML.col(
            rk.XML.cbox(label="a dialog column label")
        ),
        label="a dialog label",
        recommended=TRUE,
        i18n=list(context="context info here")
    )
    load("XML_test_standards.RData")
    expect_that(
        thisNode, equals(XML_test_standards[["dialog"]])
    )
})

test_that("rk.XML.dropdown", {
    thisNode <- rk.XML.dropdown(
        label="a dropdown label",
        options=list(
            "First Option"=c(val="val1"),
            rk.XML.option(
                "Second Option", val="val2", id.name="auto",
                i18n=list(context="context info1 here")
            ),
            "Third Option"=c(val="val3", chk=TRUE)
        ),
        i18n=list(context="context info2 here")
    )
    load("XML_test_standards.RData")
    expect_that(
        thisNode, equals(XML_test_standards[["dropdown"]])
    )
})

test_that("rk.XML.embed", {
    thisNode <- rk.XML.embed(
            component="componentID",
            button=TRUE,
            label="an embed label"
        )
    load("XML_test_standards.RData")
    expect_that(
        thisNode, equals(XML_test_standards[["embed"]])
    )
})

test_that("rk.XML.entry", {
    thisNode <- rk.XML.entry(
        component=rk.XML.component(
            label="an entry component label",
            file="plugins/MyOtherGUIdialog.xml"
        ),
        index=3
    )
    load("XML_test_standards.RData")
    expect_that(
        thisNode, equals(XML_test_standards[["entry"]])
    )
})

test_that("rk.XML.external", {
    thisNode <- rk.XML.external(
        id="externalID",
        default="none"
    )
    load("XML_test_standards.RData")
    expect_that(
        thisNode, equals(XML_test_standards[["external"]])
    )
})

test_that("rk.XML.formula", {
    thisNode <- rk.XML.formula(
        fixed=rk.XML.varslot("Fixed factors", source=test.varselector <- rk.XML.varselector("Select some vars")),
        dependent=rk.XML.varslot("Dependent variables", source=test.varselector)
    )
    load("XML_test_standards.RData")
    expect_that(
        thisNode, equals(XML_test_standards[["formula"]])
    )
})

test_that("rk.XML.frame", {
    thisNode <- rk.XML.frame(
        rk.XML.row(
            rk.XML.cbox(label="a frame row label")
        ),
        label="a frame label",
        checkable=TRUE,
        chk=FALSE
    )
    load("XML_test_standards.RData")
    expect_that(
        thisNode, equals(XML_test_standards[["frame"]])
    )
})

test_that("rk.XML.help", {
    thisNode <- rk.XML.help(
        "help_file.rkh"
    )
    load("XML_test_standards.RData")
    expect_that(
        thisNode, equals(XML_test_standards[["help"]])
    )
})

test_that("rk.XML.hierarchy", {
    thisNode <- rk.XML.hierarchy(
        rk.XML.menu("Analysis", 
            rk.XML.entry(
                rk.XML.component(
                    label="a hierarchy label",
                    file="plugins/MyOtherGUIdialog.xml"
                )
            )
        )
    )
    load("XML_test_standards.RData")
    expect_that(
        thisNode, equals(XML_test_standards[["hierarchy"]])
    )
})

test_that("rk.XML.include", {
    thisNode <- rk.XML.include(
        "../include_file.xml"
    )
    load("XML_test_standards.RData")
    expect_that(
        thisNode, equals(XML_test_standards[["include"]])
    )
})

test_that("rk.XML.input", {
    thisNode <- rk.XML.input(
        label="an input label",
        initial="init",
        size="small",
        required=TRUE,
    )
    load("XML_test_standards.RData")
    expect_that(
        thisNode, equals(XML_test_standards[["input"]])
    )
})

test_that("rk.XML.insert", {
    thisNode <- rk.XML.insert(
        snippet=rk.XML.snippet(
            rk.XML.vars(
                "Variables",
                "Fixed",
                formula.dependent="Dependent"
            )
        )
    )
    load("XML_test_standards.RData")
    expect_that(
        thisNode, equals(XML_test_standards[["insert"]])
    )
})

test_that("rk.XML.logic", {
    thisNode <- rk.XML.logic(
        rk.XML.connect(
            governor="myGovernor",
            client="myCLient"
        )
    )
    load("XML_test_standards.RData")
    expect_that(
        thisNode, equals(XML_test_standards[["logic"]])
    )
})

test_that("rk.XML.matrix", {
    thisNode <- rk.XML.matrix(
        label="a matrix label",
        mode="integer",
        rows=3,
        columns=3,
        min=0,
        max=100,
        allow_missings=TRUE,
        allow_user_resize_columns=FALSE,
        allow_user_resize_rows=FALSE,
        fixed_width=TRUE,
        fixed_height=TRUE,
        horiz_headers=c("hone", "htwo", "hthree"),
        vert_headers=c("vone", "vtwo", "vthree")
    )
    load("XML_test_standards.RData")
    expect_that(
        thisNode, equals(XML_test_standards[["matrix"]])
    )
})

test_that("rk.XML.menu", {
    thisNode <- rk.XML.menu(
        "Analysis",
        rk.XML.entry(
            rk.XML.component(
                label="a hierarchy label",
                file="plugins/MyOtherGUIdialog.xml"
            )
        ),
        index=3
    )
    load("XML_test_standards.RData")
    expect_that(
        thisNode, equals(XML_test_standards[["menu"]])
    )
})

test_that("rk.XML.optioncolumn", {
    thisNode <- rk.XML.optioncolumn(
        connect=rk.XML.input(label="an optioncolumn label"),
        modifier="text",
        label=TRUE,
        external=TRUE,
        default="rarely useful"
    )
    load("XML_test_standards.RData")
    expect_that(
        thisNode, equals(XML_test_standards[["optioncolumn"]])
    )
})

test_that("rk.XML.optiondisplay", {
    thisNode <- rk.XML.optiondisplay(
        index=FALSE
    )
    load("XML_test_standards.RData")
    expect_that(
        thisNode, equals(XML_test_standards[["optiondisplay"]])
    )
})

test_that("rk.XML.option", {
    thisNode <- rk.XML.option(  
        label="an option label",
        val="value",
        chk=TRUE
    )
    load("XML_test_standards.RData")
    expect_that(
        thisNode, equals(XML_test_standards[["option"]])
    )
})

test_that("rk.XML.optionset", {
    thisNode <- rk.XML.optionset(
        content=list(
            rk.XML.row(
                rk.XML.cbox(label="a content row label")
            )
        ),
        optioncolumn=list(
            rk.XML.optioncolumn("an optioncolumn")
        ),
        min_rows=1,
        min_rows_if_any=1,
        max_rows=5,
        keycolumn="myKey",
        logic=rk.XML.logic(
            rk.XML.connect(
                governor="myGovernor",
                client="myCLient"
            )
        ),
        optiondisplay=TRUE
    )
    load("XML_test_standards.RData")
    expect_that(
        thisNode, equals(XML_test_standards[["optionset"]])
    )
})

test_that("rk.XML.page", {
    thisNode <- rk.XML.page(
        rk.XML.text("a page text")
    )
    load("XML_test_standards.RData")
    expect_that(
        thisNode, equals(XML_test_standards[["page"]])
    )
})

# test_that("", {
#     thisNode <- 
#     load("XML_test_standards.RData")
#     expect_that(
#         thisNode, equals(XML_test_standards[[""]])
#     )
# })
