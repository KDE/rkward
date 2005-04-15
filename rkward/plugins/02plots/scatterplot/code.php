<?
	function preprocess () {
	}
?>
<?

	function calculate () {
$x = str_replace ("\n", ",", trim (getRK_val ("x"))) ;
$y = str_replace ("\n", ",", trim (getRK_val ("y"))) ;

?>


<? #input ?>
rk.plugin.Xvar = list(<? echo ($x) ;?>) ; 
rk.plugin.Yvar = list(<? echo ($y) ;?>) ; 
rk.plugin.Xval =  c(<? echo ($x) ;?>) ;
rk.plugin.Yval =  c(<? echo ($y) ;?>) ;


<? # verification (chiant mais doit être fait)?>
rk.plugin.ok = TRUE ;
if (length(rk.plugin.Xvar) != length(rk.plugin.Yvar) ) { 
	rk.plugin.ok = FALSE ;
	stop("'X' is of length ",length(rk.plugin.Xvar)," and 'Y' of length ",length(rk.plugin.Yvar) )
	}
 if ( "<? getRK("color") ; ?>" == "each") {
	if (length( <? getRK('col') ; ?>) != length(rk.plugin.Xvar))
	{
		rk.plugin.ok = FALSE ;
		stop('only ', length( <? getRK('col') ; ?>) ,' color(s) is(are) displayed') ;
	}
}
if ( "<? getRK("isPch") ; ?>" == "each") {
	if (length( <? getRK('pch') ; ?>) != length(rk.plugin.Xvar))
	{
		rk.plugin.ok = FALSE ;
		stop('only ', length( <? getRK('pch') ; ?>) ,' symbol(s) is(are) displayed') ;
	}
}
if ( "<? getRK("isCex") ; ?>" == "each") {
	if (length( <? getRK('cex') ; ?>) != length(rk.plugin.Xvar))
	{
		rk.plugin.ok = FALSE ;
		stop('only ', length( <? getRK('pch') ; ?>) ,' size(s) is(are) displayed') ;
	}
}
if ( "<? getRK("type") ; ?>" == "custoType") {
	if (length( <? getRK('typeCusto') ; ?>) != length(rk.plugin.Xvar))
	{
		rk.plugin.ok = FALSE ;
		stop('only ', length( <? getRK('typeCusto') ; ?>) ,' type(s) is(are) displayed') ;
	}
}


if (rk.plugin.ok) {

<? #finding min and max for default plotin  ; ?>
<?  if (getRK_val("columns") == "custoCol" ) echo (getRK("Xscale") . "-> rk.plugin.Xval") ; ?> 
<?  if (getRK_val("rows") == "custoRow" ) echo (getRK("Yscale") . "-> rk.plugin.Yval") ; ?>
rk.plugin.Xdef = c(min(rk.plugin.Xval,na.rm=TRUE) , max(rk.plugin.Xval,na.rm=TRUE))
rk.plugin.Ydef = c(min(rk.plugin.Yval,na.rm=TRUE) , max(rk.plugin.Yval,na.rm=TRUE))

<? # names ?>
rk.plugin.Xname = '<? getRK("Xname") ?>' ;
rk.plugin.Yname = '<? getRK("Yname") ?>' ;
rk.plugin.title =  '<? getRK("main") ?>' ;
rk.plugin.sub =  '<? getRK("sub") ?>' ;
<? if  (getRK_val("isXaxis") != "1" ) echo ( "rk.plugin.Xname  = '' "  ) ?> 
<? if  (getRK_val("isYaxis") != "1" ) echo ( "rk.plugin.Yname  = '' "  ) ?> 
<? if  (getRK_val("isSub") != "1" ) echo ( "rk.plugin.sub  = '' "  ) ?> 
<? if  (getRK_val("isTitle") != "1" ) echo ( "rk.plugin.main  = '' "  ) ?> 


<? # type ?>
rk.plugin.tc = data.frame(
type = rep(NA,length(rk.plugin.Xvar)),
col = rep(NA,length(rk.plugin.Xvar)),
pch = rep(NA,length(rk.plugin.Xvar)),
cex = rep(NA,length(rk.plugin.Xvar)))

<? if  (getRK_val("type") != "custoType" ) echo ( getRK("type") . " ->  rk.plugin.tc[[1]]" ) ; 
else echo( getRK("typeCusto") . " ->  rk.plugin.type.tc[[1]]"  ) ?> 
rk.plugin.tc[[2]] = <? getRK("col")  ;?> 
rk.plugin.tc[[3]] = <? getRK("cex")  ;?> 
rk.plugin.tc[[4]] = <? getRK("pch")  ;?> 

<? # avant après ?>
<? /* TODO 
rk.plugin.on = expression( <? getRK("rkgraphson") ; ?>) ;
rk.plugin.off = expression( <? getRK("rkgraphsoff") ; ?>) ;
*/ ?>
rk.plugin.before = expression( <? getRK("before") ; ?>) ;
rk.plugin.after = expression( <? getRK("after") ; ?>) ;

<? # axes ?>
rk.plugin.axes = <? getRK("axes") ;?> 
rk.plugin.log = '<? getRK("logX") ; getRK("logY") ; ?>'


}
<?

	}

	?>
	<?

	function printout () {
	
?>
if (rk.plugin.ok) {

rk.graph.on()
# evaluating before
# doesn't work very well 
#if (!is.null(eval(rk.plugin.on))) eval(rk.plugin.on)
rk.graph.on()
if (!is.null(eval(rk.plugin.before))) eval(rk.plugin.before)

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

# evaluating after
if (!is.null(eval(rk.plugin.after))) eval(rk.plugin.after)
#doesn't work very well
#if (!is.null(eval(rk.plugin.off))) eval(rk.plugin.off)
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
