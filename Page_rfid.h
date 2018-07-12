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
<tr><td align="right">Estado Parada1 :</td><td><span id="parada1"></span></td></tr>
<tr><td align="right">Estado Parada2 :</td><td><span id="parada2"></span></td></tr>
<tr><td align="right">Estado Parada3 :</td><td><span id="parada3"></span></td></tr>

<tr><td colspan="2"><hr></span></td></tr>

<tr><td colspan="2" align="center"><a href="javascript:GetState()" class="btn btn--m btn--blue">Refresh</a></td></tr>
</table>
<script>

function GetState()
{
  setValues("/rfidvalues");
}

function refreshData(){
  GetState();
  setTimeout(refreshData, 31000);
}

window.onload = function ()
{
  load("style.css","css", function() 
  {
    load("microajax.js","js", function() 
    {
        refreshData();
    });
  });
}
function load(e,t,n){if("js"==t){var a=document.createElement("script");a.src=e,a.type="text/javascript",a.async=!1,a.onload=function(){n()},document.getElementsByTagName("head")[0].appendChild(a)}else if("css"==t){var a=document.createElement("link");a.href=e,a.rel="stylesheet",a.type="text/css",a.async=!1,a.onload=function(){n()},document.getElementsByTagName("head")[0].appendChild(a)}}



</script>
)=====" ;

//<meta http-equiv="refresh" content="30" > Adding this to the HTML will make the page reload every 30 secs

 void send_RFID_PageOrTake_Post(){
  // Requiere: urldecode
  // Efect: parse incoming post or get values
  // Modifies: clients.parada1, 2 or 3 data structure (declared on global.h)
   if (server.args() > 0 ){ // If the server sent arguments then...
    clients tempClient;
    String clientstr(F("client"));
    String statustr(F("status"));
    String Parada1str(F("Parada1"));
    String Parada2str(F("Parada2"));
    //String Parada3str(F("Parada3"));
     for ( uint8_t i = 0; i < server.args(); i++ ) { // For each argument sent by the client parse it into the correct variable
        if (server.argName(i) == clientstr) tempClient.name = urldecode( server.arg(i)); 
        if (server.argName(i) == statustr) tempClient.status = urldecode( server.arg(i));
      }
      if (tempClient.name == Parada1str){ // Check from which client the data comes from
       Parada1.name = tempClient.name;
       Parada1.status = tempClient.status;
       Serial.println(F("POST Parada1"));
      } else if (tempClient.name == Parada2str){
       Parada2.name = tempClient.name;
       Parada2.status = tempClient.status;
       Serial.println(F("POST Parada2"));
      } else {
       Parada3.name = tempClient.name;
       Parada3.status = tempClient.status;
       Serial.println(F("POST Parada3")); 
      }
      tempClient.name ="";
      tempClient.status="";
   }
   server.send ( 200, "text/html", PAGE_RFID ); // return the PAGE_RFID html
   Serial.println(__FUNCTION__); 
   
 }


void send_rfid_values_html ()// FILL WITH INFOMATION
{

  String values ="";
  values += "parada1|" +  (String) Parada1.status + "|div\n";
  values += "parada2|" +  (String) Parada2.status + "|div\n";
  values += "parada3|" +  (String) Parada3.status + "|div\n";
  server.send ( 200, "text/plain", values);
  Serial.println(__FUNCTION__);
  values ="";

}

#endif
