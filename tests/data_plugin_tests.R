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
		})
	), postCalls = list (
			function(){rm("women", pos=globalenv())}
			)	# like initCalls: run after all tests to clean up. Empty in this case.
)
