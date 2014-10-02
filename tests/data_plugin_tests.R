## definition of the test suite
suite <- new ("RKTestSuite", id="data_plugin_tests",
	# place here libraries that are required for *all* tests in this suite, or highly likely to be installed
	libraries = c ("datasets"),
	# initCalls are run *before* any tests. Use this to set up the environment
	initCalls = list (
		function () {
			# prepare some different files for loading
			library ("datasets")
			data (women)
			data (sleep)
			data (warpbreaks)
			withnas <- sleep$ID
			withnas[c(3,15)] <- NA
		}
	## the tests
	), tests = list (
		new ("RKTest", id="sort_data", call=function () {
			assign("women.backup", datasets::women, pos=globalenv())
			rk.sync.global()

			rk.call.plugin ("rkward::sort_data", conversion.string="as is", multi_sortby.serialized="keys=women.backup[[\\\"height\\\"]]\n_row=conversion.string=as is\treverse.state=1", object.available="women.backup", order.string="", saveto_select.string="same", sortby.available="women.backup[[\"height\"]]", submit.mode="submit")

			stopifnot (all.equal (women.backup, women[order (-women$height),]))
		}),
		new ("RKTest", id="subset_dataframe", call=function () {
			rk.call.plugin ("rkward::subset_dataframe", drp_fltr_num.string="range", frm_Onlyssbs.checked="1", inp_Exprssnr.text="group == 1", inp_Mnmmrmpt.text="0", inp_Mxmmrmpt.text="3", maxinc.state="0", mininc.state="1", svb_Svrsltst.active="1", svb_Svrsltst.objectname="sset.result", svb_Svrsltst.parent=".GlobalEnv", var_data.available="sleep", vrsl_Fltrbyvr.available="sleep[[\"extra\"]]", vrsl_Slctdvrb.available="sleep[[\"extra\"]]\nsleep[[\"ID\"]]", submit.mode="submit")
		}),
		new ("RKTest", id="recode_cateorigal", call=function () {
		        rk.call.plugin ("rkward::recode_categorical", datamode.string="factor", other.string="copy", saveto.objectname="recoded", saveto.parent=".GlobalEnv", saveto_select.string="other", set.serialized="_row=new_value.string=custom\tnew_value_custom.input.text=low\told_value_type.string=value\tvalues.available=\\\"L\\\"\n_row=new_value.string=custom\tnew_value_custom.input.text=midorhigh\told_value_type.string=value\tvalues.available=\\\"M\\\"\\n\\\"H\\\"", x.available="warpbreaks[[\"tension\"]]", submit.mode="submit")
			rk.print (recoded)
			rm (recoded, envir=.GlobalEnv)
			rk.call.plugin ("rkward::recode_categorical", datamode.string="logical", other.string="na", saveto.objectname="recoded2", saveto.parent=".GlobalEnv", saveto_select.string="other", set.serialized="_row=new_value.string=custom\tnew_value_custom.logical.string=FALSE\told_value_type.string=value\tvalues.available=\\\"2\\\"\\n\\\"3\\\"\\n\\\"4\\\"\\n\\\"5\\\"\\n\\\"6\\\"\\n\\\"7\\\"\\n\\\"8\\\"\\n\\\"9\\\"\\n\\\"10\\\"\n_row=new_value.string=na\told_value_type.string=value\tvalues.available=\\\"9\\\"\\n\\\"10\\\"\n_row=new_value.string=custom\tnew_value_custom.logical.string=TRUE\told_value_type.string=na", x.available="withnas", submit.mode="submit")
			rk.print (recoded2)
			rm (recoded2, envir=.GlobalEnv)
		})
	), postCalls = list (
			function(){rm("women", pos=globalenv())}
			)	# like initCalls: run after all tests to clean up. Empty in this case.
)
