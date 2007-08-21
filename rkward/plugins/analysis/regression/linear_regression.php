<?
function preprocess () { ?>
require(xtable)
<?        }

function calculate () {
?>
# This can be used in a later fase to give the user the option to choose 
# precision. For the moment is stable 5 (In summary.lm is 4).
n=5
results <- summary.lm (lm (<? getRK ("y"); ?> ~ <? getRK ("x"); ?>) )
# Here we get the quantiles of the residuals and place them in a matrix 
# and we name them in order to use xtable
quarlm<-matrix(quantile(residuals(results)),nrow=1, ncol=5)
colnames(quarlm)<-c("Min","1Q","Median","2Q","Max")
# We extract the coefficients
coeflm<-coef(results)
# the Residual standard error
siglm<-results$sigma
# the maximum degrees of fredom
maxdflm<-(max(results$df))
# the r squared and the adjusted r squared
rsq<-results$r.squared
arsq<-results$adj.r.squared
# the fstatistic, we put it into a matrix and we name it in order to use xtable 
fslm<-matrix(results$fstatistic,nrow=1,ncol=3)
colnames(fslm)<-names(results$fstatistic)
<?
}

function printout () {
?>
rk.header ("Linear Regression")
rk.print("Residuals")
rk.results(xtable(quarlm))
rk.print("Coefficients")
rk.results(xtable(coeflm))
rk.print(c("Residual standard error: ", round(siglm,n), " on ", maxdflm, " degrees of fredom"))
rk.print(c("Multiple R-Squared: ", round(rsq,n), ", Adjusted R-squared: ", round(arsq,n)))
rk.print("F-statistic")
rk.results(xtable(fslm))
<?
}
?>
