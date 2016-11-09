<?php
include("functions.inc.php");

$aStatus=array();
$aStatus['iErrorCode']=0;
$aStatus['sErrorText']="";


$iParam=intval($_GET['iDirection']);

//status abfragen
$iStatus=sSendRequest($iParam);
$aStatus['iStatusId']=$iStatus;
$aStatus['sStatusText']=sGetStatusText($iStatus);




echo json_encode($aStatus);
?>
