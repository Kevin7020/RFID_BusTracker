#ifndef PAGE_RFID_H
#define PAGE_RFID_H


//
//   The HTML PAGE
//
const char PAGE_RFID[] PROGMEM = R"=====(
<meta name="viewport" content="width=device-width, initial-scale=1" />
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<a href="/admin.html"  class="btn btn--s">Settings</a>&nbsp;&nbsp;<strong>RFID Information</strong>
<hr>
<table border="0"  cellspacing="0" cellpadding="3" style="width:310px" >
<tr><td align="right">Estado Parada :</td><td><span id="parada"></span></td></tr>

<tr><td colspan="2"><hr></span></td></tr>

<tr><td colspan="2" align="center"><a href="javascript:GetState()" class="btn btn--m btn--blue">Refresh</a></td></tr>
</table>
<script>

function GetState()
{
  setValues("/admin/rfidvalues");
}

window.onload = function ()
{
  load("style.css","css", function() 
  {
    load("microajax.js","js", function() 
    {
        GetState();
    });
  });
}
function load(e,t,n){if("js"==t){var a=document.createElement("script");a.src=e,a.type="text/javascript",a.async=!1,a.onload=function(){n()},document.getElementsByTagName("head")[0].appendChild(a)}else if("css"==t){var a=document.createElement("link");a.href=e,a.rel="stylesheet",a.type="text/css",a.async=!1,a.onload=function(){n()},document.getElementsByTagName("head")[0].appendChild(a)}}



</script>
)=====" ;

/*
 *void send_RFID_PageOrTake_Post()
 *{
 *  
 *   
 *  if (server.args() > 0 )  // Save Settings
 *  {
 *
 *    // Requiere: urldecode
 *    // Efect: parse incoming post or get values
 *    // Modifies: here(int) and ClientName(str) (declared on global.h)
 *
 *    for ( uint8_t i = 0; i < server.args(); i++ ) {
 *      if (server.argName(i) == "ClientName") ClientName = urldecode( server.arg(i)); 
 *      if (server.argName(i) == "here") here =  server.arg(i).toInt();
 *      }
 *
 *    //Serial.println("Server= ");
 *    //Serial.println(ServerName);
 *    //Serial.println("Here= ");
 *    //Serial.println(here);
 *
 *  }
 *  server.send ( 200, "text/html", PAGE_RFID ); 
 *  Serial.println(__FUNCTION__); 
 *  
 *}
*/

void send_rfid_values_html ()// FILL WITH INFOMATION
{

  String values ="";
  values += "parada|" +  (String) parada.status + "|div\n";
  server.send ( 200, "text/plain", values);
  values ="";
  Serial.println(__FUNCTION__); 

}

#endif
