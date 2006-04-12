<?
	function preprocess () {
	}

	function calculate () {
$x = str_replace ("\n", ",", trim (getRK_val ("x"))) ;
$y = str_replace ("\n", ",", trim (getRK_val ("y"))) ;

/** fetch some values which are needed in more than one place, to avoid mulitple transfer */
$type = getRK_val ("type");
$typeCusto = getRK_val ("typeCusto");
$col = getRK_val ("col");
$pch = getRK_val ("pch");
$cex = getRK_val ("cex");
if (getRK_val("isXaxis") == "1") $Xname = getRK_val ("Xname"); else $Xname = "";
if (getRK_val("isYaxis") == "1") $Yname = getRK_val ("Yname"); else $Yname = "";
if (getRK_val("isTitle") == "1") $main = getRK_val ("main"); else $main = "";
if (getRK_val("isSub") == "1") $sub = getRK_val ("sub"); else $sub = "";
?>


<? #input ?>
rk.plugin.Xvar <- list(<? echo ($x) ;?>)
rk.plugin.Yvar <- list(<? echo ($y) ;?>)
rk.plugin.Xval <- <? if (getRK_val("columns") == "custoCol" ) echo (getRK_val("Xscale") . "\n"); else echo ("c(" . $x . ")\n"); ?>
rk.plugin.Yval <- <? if (getRK_val("rows") == "custoRow" ) echo (getRK_val("Yscale") . "\n"); else echo ("c(" . $y . ")\n"); ?>

<? # verification (chiant mais doit être fait)?>
rk.plugin.ok = TRUE ;
if (length(rk.plugin.Xvar) != length(rk.plugin.Yvar) ) { 
	rk.plugin.ok = FALSE ;
	stop("'X' is of length ",length(rk.plugin.Xvar)," and 'Y' of length ",length(rk.plugin.Yvar) )
}
<?
if (getRK_val ("color") == "each") { ?>
if (length( <? echo ($col); ?>) != length(rk.plugin.Xvar)) {
	rk.plugin.ok = FALSE ;
	stop('only ', length( <? echo ($col); ?>) ,' color(s) is(are) displayed') ;
}
<? }
if (getRK_val ("isPch") == "each") { ?>
if (length( <? echo ($pch); ?>) != length(rk.plugin.Xvar)) {
	rk.plugin.ok = FALSE ;
	stop('only ', length( <? echo ($pch); ?>) ,' symbol(s) is(are) displayed') ;
}
<? }
if (getRK_val ("isCe") == "each") { ?>
if (length( <? echo ($cex); ?>) != length(rk.plugin.Xvar)) {
	rk.plugin.ok = FALSE ;
	stop('only ', length( <? echo ($cex); ?>) ,' size(s) is(are) displayed') ;
}
<? }
if ($type == "custoType") { ?>
if (length( <? echo ($typeCusto); ?>) != length(rk.plugin.Xvar)) {
	rk.plugin.ok = FALSE ;
	stop('only ', length( <? echo ($typeCusto); ?>) ,' type(s) is(are) displayed') ;
}
<? } ?>

if (rk.plugin.ok) {

<? #finding min and max for default plotin  ; ?>
rk.plugin.Xdef = c(min(rk.plugin.Xval,na.rm=TRUE) , max(rk.plugin.Xval,na.rm=TRUE))
rk.plugin.Ydef = c(min(rk.plugin.Yval,na.rm=TRUE) , max(rk.plugin.Yval,na.rm=TRUE))

<? # names ?>
rk.plugin.Xname = '<? echo ($Xname); ?>'
rk.plugin.Yname = '<? echo ($Yname); ?>'
rk.plugin.title = '<? echo ($main); ?>'
rk.plugin.sub = '<? echo ($sub); ?>'

<? # type ?>
rk.plugin.tc = data.frame(
type = rep(NA,length(rk.plugin.Xvar)),
col = rep(NA,length(rk.plugin.Xvar)),
pch = rep(NA,length(rk.plugin.Xvar)),
cex = rep(NA,length(rk.plugin.Xvar)))

<? if  ($type != "custoType" ) echo ( $type . " ->  rk.plugin.tc[[1]]" ) ;
else echo( $typeCusto . " ->  rk.plugin.type.tc[[1]]"  ) ?>
rk.plugin.tc[[2]] = <? echo ($col); ?>
rk.plugin.tc[[3]] = <? echo ($cex); ?>
rk.plugin.tc[[4]] = <? echo ($pch); ?>

<? # avant après ?>
<? /* TODO 
rk.plugin.on = expression( <? getRK("rkgraphson") ; ?>) ;
rk.plugin.off = expression( <? getRK("rkgraphsoff") ; ?>) ;
*/ ?>

<? # axes ?>
rk.plugin.axes = <? getRK("axes") ;?> 
rk.plugin.log = '<? getRK("logX") ; getRK("logY") ; ?>'


}
<?

	}

	function printout () {
	
?>
if (rk.plugin.ok) {

rk.graph.on()
<? 
$before = getRK_val ("before");
if (!empty ($before)) echo ($before);
?>

# making frame 
plot(rk.plugin.Xdef,rk.plugin.Ydef,type="n" , xlab = rk.plugin.Xname , ylab = rk.plugin.Yname , main = rk.plugin.title , sub = rk.plugin.sub , axes = rk.plugin.axes , log = rk.plugin.log)

# ploting 
for (rk.plugin.iterator in 1:length(rk.plugin.Xvar)) {
	points  (
		rk.plugin.Xvar[[rk.plugin.iterator]] ,
		rk.plugin.Yvar[[rk.plugin.iterator]] , 
		type = rk.plugin.tc[[1]][[rk.plugin.iterator]] , 
		col = rk.plugin.tc [[2]] [[rk.plugin.iterator]] ,
		cex = rk.plugin.tc [[3]] [[rk.plugin.iterator]] ,
		pch = rk.plugin.tc [[4]] [[rk.plugin.iterator]] 
		)
}

<?
$after = getRK_val ("after");
if (!empty ($after)) echo ($after);
?>

<? /*#doesn't work very well
#if (!is.null(eval(rk.plugin.off))) eval(rk.plugin.off) */ ?>
rk.graph.off()
}

<?
	}
	
	function cleanup () {
?>

rk.plugin.remove = ls() [grep('rk.plugin',ls())]
rm(list=rk.plugin.remove)
rm(rk.plugin.remove)
 
<?
	}
?>
