<?
function preprocess () {
}

function calculate () {
	global $envir;

	if (getRK_val ("other_env")) {
		$other_env = true;
		$envir = getRK_val ("envir");
	} else {
		$envir = "globalenv()";
	}

	if ($other_env) { ?>
assign ("<? echo ($envir); ?>, new.env (parent=globalenv()), envir=globalenv())
<?	} ?>
load (file="<? getRK("file"); ?>", envir=<? echo ($envir); ?>)
<?
}

function printout () {
	global $envir; 
	makeHeaderCode ("Load data", array ("File" => getRK_val ("file"), "Import to environment" => $envir));
}

?>
