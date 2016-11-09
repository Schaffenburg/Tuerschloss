<?php
function sSendRequest($iRequestCode){
$sRequestCommand="0";
if($iRequestCode==1){
	$sRequestCommand="open";
}else if($iRequestCode==2){
	$sRequestCommand="close";
}
exec("/etc/skripte/i2cCommand $sRequestCommand",$response);
return $response[sizeof($response)-1];
}

function sGetStatusText($iStatusCode){
$sStatusText="";
switch($iStatusCode){
        case 0: $sStatusText="unterwegs";break;
        case 1: $sStatusText="verriegelt";break;
        case 2: $sStatusText="offen";break;
        case 3: $sStatusText="warten auf verriegeln";break;

}
return $sStatusText;
}
?>
