<?php
//**************************************************************//
//  Name    : gatekeeper                                        //
//  Author  : Karsten Schlachter                                //
//  Date    : Oktober 2015 letztes update Nov 16                //
//  Version : 1.2                                               //
//  Notes   : Website zu Tuersteuerung                          //
//  Web   : https://wiki.schaffenburg.org/Projekt:Tuerschloss   // 
//****************************************************************
include("functions.inc.php");
?>
<!doctype html>
<html>
<head>
    <title>My Page</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="stylesheet" href="https://code.jquery.com/mobile/1.4.5/jquery.mobile-1.4.5.min.css">
    <script src="https://code.jquery.com/jquery-2.1.4.min.js"></script>
    <script src="https://code.jquery.com/mobile/1.4.5/jquery.mobile-1.4.5.min.js"></script>
</head>
<body>
<!--
 https://wiki.schaffenburg.org/Projekt:Tuerschloss 
-->
    <div data-role="page" >
 
        <div data-role="header">
            <h1>Schaffenburg</h1>
        </div><!-- /header -->
 
        <div role="main" class="ui-content" style="text-align:center;">
<img src="schaffenburg_wappen.png">			
<form><label for="flip-2">T&uuml;rstatus</label>
			<select name="lockSwitch" data-role="slider" data-track-theme="b" data-theme="b" id="lockSwitch">
			<option value="locked"> verriegelt </option>
			<option value="open">offen</option>
			</select>
			</form>
        </div><!-- /content -->
 
        <div data-role="footer">
            <h4>Schaffenburg e.V.</h4>
        </div><!-- /footer -->
 
    </div><!-- /page -->
<script>
$(function(){
var iDirection=0;
var iCurDirection=<?=intval(sSendRequest(0))?>;
var iLastLock=0;

if(iCurDirection===2){
	$("#lockSwitch").val("open").slider("refresh");
}
$("#lockSwitch").change(function(){
//	console.log("test "+$(this).val());

	if($(this).val()==="open"){
	//TODO fuer aufschliessen zunaechst nach code fragen
	
		if((Date.now()-iLastLock)<1000*5){//innerhalb von 5s wieder zuruecksetzen
			$(this).val("locked");
			return;
		}else if((Date.now()-iLastLock)<1000*60){//innerhalb von 1min explizit fragen
		var response=prompt("Tür wirklich wieder aufschließen? 'ja' zum bestätigen","");
			if(!(response=="ja" ||response=="Ja")){
				$(this).val("locked");
				return;
			}
		}
	iDirection=1;
	}else{
		iDirection=2;
		iLastLock=Date.now();
	}
	$.getJSON( "https://door.schaffenburg.org/json.php?u="+(new Date().getTime()),{'iDirection':iDirection}, function( data ) {

//	console.log(data);
	});

});

});
</script>
</body>
</html>
