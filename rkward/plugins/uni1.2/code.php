<?
	function preprocess () {
	}
	
	function calculate () {
$vars = "substitute (" . str_replace ("\n", "), substitute (", trim (getRK_val ("z"))) . ")";
?>
# we make the calculation
rk.temp.res <- list()
rk.temp.option <- NA
for (rk.temp.var in list (<? echo ($vars); ?>))  {
	k <-  rk.get.description(rk.temp.var, is.substitute=TRUE)
	rk.temp.var <- eval(rk.temp.var)
	rk.temp.res [[ k ]] <- list() 
	<? if (getRK_val ("nombre")) 
		echo "rk.temp.res [[ k ]][['Number of obs']] <- length(rk.temp.var)\n";
	if (getRK_val ("nbna")) 
		echo "rk.temp.res [[ k ]][['Number of missing values']] <- length(which(is.na(rk.temp.var)))\n" ;
	if (getRK_val ("moyenne")) 
		echo "rk.temp.res [[ k ]][['Mean']] <- mean(rk.temp.var,na.rm=".getRK_val("NA").")\n" ;
	if (getRK_val ("vari")) 
		echo "rk.temp.res [[ k ]][['Variance']] <- var(rk.temp.var,na.rm=".getRK_val("NA").")\n" ;
	if (getRK_val ("ecartt")) 
		echo "rk.temp.res [[ k ]][['Sd']] <- sd(rk.temp.var,na.rm=".getRK_val("NA").")\n" ;
	if (getRK_val ("minimum")) 
		echo "rk.temp.res [[ k ]][['Minimum']] <- min(rk.temp.var,na.rm=".getRK_val("NA").")\n" ;
	if (getRK_val ("maximum")) 
		echo "rk.temp.res [[ k ]][['Maximum']] <- max(rk.temp.var,na.rm=".getRK_val("NA").")\n" ;
	if (($nmin = getRK_val ("nbminimum")) != "0") 
		echo (" if ( length(rk.temp.var) >= " . $nmin .") {
		rk.temp.res [[ k ]][['Minimum values']] <- list()
		rk.temp.res [[ k ]][['Minimum values']][[1]] <- c(1:" . $nmin . ")
		rk.temp.res [[ k ]][['Minimum values']][[2]] <- sort(rk.temp.var, decreasing=FALSE,na.last=TRUE) [1:" . $nmin  .   "]
		}\n"  ) ;
	if (($nmax = getRK_val ("nbmaximum")) != "0") 
		echo (" if ( length(rk.temp.var) >= " . $nmax .") 
		rk.temp.res [[ k ]][['Maximum values']] <- list()
		rk.temp.res [[ k ]][['Maximum values']][[1]] <- c(1:" . $nmax . ")
		rk.temp.res [[ k ]][['Maximum values']][[2]] <- sort(rk.temp.var, decreasing=TRUE,na.last=TRUE) [1:" . $nmax  .   "]\n"  ) ;
	if (getRK_val ("mediane")) echo "rk.temp.res [[ k ]][['Median']] <- median(rk.temp.var,na.rm=".getRK_val("NA").")\n" ;
	if (getRK_val ("irq")) echo "rk.temp.res [[ k ]][['Inter Quartile Range']] <- IQR(rk.temp.var,na.rm=".getRK_val("NA").")\n";
	if (getRK_val ("quartile")) echo 
		"rk.temp.res [[ k ]] [['Quartiles']]  <- list()
		rk.temp.res [[ k ]] [['Quartiles']] [[2]]  <- quantile(rk.temp.var,na.rm=".getRK_val("NA").")
		rk.temp.res [[ k ]] [['Quartiles']] [[1]]  <- names(quantile(rk.temp.var,na.rm=".getRK_val("NA")."))\n"  ;
	if (($nautre = getRK_val ("autre")) != "0") echo (" if ( length(rk.temp.var) >= " . $nautre .") {
		rk.temp.res [[ k ]][['Other']] <- list()
		rk.temp.res [[ k ]][['Other']][[1]] <- paste(seq(0,100,le=" . $nautre . "),'%')
		rk.temp.res [[ k ]] [['Other']] [[2]]  <- quantile(rk.temp.var,probs=seq(0,1,le=" . $nautre . "), na.rm=".getRK_val("NA").")
		}\n"  ) ; ?> 
	
	#robust statistics
	<?  if (getRK_val ("trim") == "1") 
		echo ("rk.temp.res [[ k ]][['Trimmed Mean']] <- mean(rk.temp.var,trim= ". getRK_val("pourcent") . " ,na.rm=".getRK_val("NA").")\n" ) ;
	if (getRK_val ("mad") == "1") 
		echo ("rk.temp.res [[ k ]][['Median Absolute Deviation']] <-  mad(rk.temp.var, constant = ". getRK_val("constMad") . " ,na.rm=".getRK_val("NA").")\n" ) ; 
	if (getRK_val ("huber") == "1") echo ("
		require (\"MASS\")
		rk.temp.res [[ k ]][['Huber M-Estimator']] <- list()
		rk.temp.res [[ k ]][['Huber M-Estimator']] [[1]] <- c('Location Estimate','Mad scale estimate')
		rk.temp.res [[ k ]][['Huber M-Estimator']] [[2]] <- c(NA,NA)
		try(rk.temp.res [[ k ]][['Huber M-Estimator']] [[2]] <- hubers (rk.temp.var, k = " . getRK_val("winsor") . ",tol=".getRK_val("tol")." )\n");
	if (getRK_val(customMu)=="1") echo (",mu=".getRK_val("mu")) ; 
	if (getRK_val(customS)=="1") echo (",s=".getRK_val("s")) ;
	if (getRK_val ("huber") == "1") echo (",initmu =".getRK_val("initmu")."(rk.temp.var)))")
	?>	
	rm(k)
}

	
	<? if (getRK_val ("result") == "1") echo( getRK_val("nom")."<- rk.temp.res") ?> 
	<? getRK_val("option") ?> 
	<? if (getRK_val("option")=="1") echo("
	rk.temp.option <- list()
	rk.temp.option [['Remove missing value from calcul ']] <- paste( 'Remove missing value from calcul',". getRK_val("NA") . ",sep=' = ')" )  ?> 
	<? if (getRK_val("option")=="1" && getRK_val("trim")=="1" ) echo("rk.temp.option [['Trimmed value for trimmed mean']] <-  paste('Trimmed value for trimmed mean',". getRK_val("pourcent") . ",sep=' = ')"); ?> 
	<? if (getRK_val("option")=="1" && getRK_val("mad")=="1" ) echo("rk.temp.option [['Constant for the MAD estimation ']] <-  paste('Constant for the MAD estimation '," . getRK_val("constMad") . ",sep=' = ')" ) ?> 
	<? if (getRK_val("option")=="1" && getRK_val("huber")=="1" ) echo("
	rk.temp.option [['Winsorized values for Huber estimator ']] <- paste('Winsorized values for Huber estimator'  ," . getRK_val("winsor") . ",sep=' = ')" ."
	rk.temp.option [['Tolerance in Huber estimator ']] <-  paste( 'Tolerance in Huber estimator '," .getRK_val("tol") . ",sep=' = ')" )?> 
	<? if (getRK_val("option")=="1" && getRK_val("huber")=="1" && getRK_val("customMu")=="1" ) echo ("rk.temp.option [['Mu for Huber estimator ']] <-  paste('Mu for Huber estimator' ,".getRK_val("mu") . ",sep=' = ')" )?>  
	<? if (getRK_val("option")=="1" && getRK_val("huber")=="1" && getRK_val("customS")=="1" ) echo ("rk.temp.option [['S for Huber estimator ']] <-  paste('S for Huber estimator' ,".getRK_val("s")   . ",sep=' = ')" )?>  
	<? if (getRK_val("option")=="1" && getRK_val("huber")=="1" ) echo ("rk.temp.option [['Inial value']] <-  paste('Inial value' ,'".getRK_val("initmu") . "',sep=' = ')" ) ?> 
	
	

<?
	}
	
	function printout () {
	// produce the output
?>

cat(paste("<h1>Univariate statistics of ","</h1>\n",sep=""))

for (rk.temp.var in names(rk.temp.res )) {
	cat(paste("<h1>Univariate statistics of ",rk.temp.var,"</h1>\n",sep=""))
	cat ("<table border=\"1\">")
	for (i in names(rk.temp.res[[rk.temp.var]])) {
	if (i %in% c('Quartiles','Minimum values','Maximum values','Other','Huber M-Estimator'))
	{
		cat(paste("<tr><td>",i,"</td></tr>\n",sep=""))
		cat(paste("<tr><td>",rk.temp.res[[rk.temp.var]][[i]][[1]],"</td><td>",rk.temp.res[[rk.temp.var]][[i]][[2]],"</td></tr>\n",sep=""))
	}
	else
		cat(paste("<tr><td>",i ,"</td><td>",rk.temp.res[[rk.temp.var]][[i]],"</td></tr>\n",sep=""))
	}
	cat("</table>")
}
if ( ! is.na(rk.temp.option[[1]])) rk.print(rk.temp.option)

<?
	}
	
	function cleanup () {
?>

rm(rk.temp.res,rk.temp.option,rk.temp.var)

<?
	}
?>
