#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__
void http_server_netconn_init(void);
void http_paser(char *buf);

#define HTMLCODE "<!DOCTYPE html>\
<html>\
<head>\
<style>\
alinear_wifi {\
margin-right: 150px;\
margin-left: 33px;\
}\
</style>\
</head>\
<style>\
alinear_ruta {\
margin-right: 150px;\
margin-left: 21px;\
}\
</style>\
</head>\
<head>\
<style>\
alinear_precio {\
margin-right: 150px;\
margin-left: 7px;\
}\
</style>\
</head>\
<head>\
<style>\
alinear_mail {\
margin-right: 150px;\
margin-left: 21px;\
}\
</style>\
</head>\
<head>\
<style>\
.form-inline button {\
padding: 10px 20px;\
background-color: dodgerblue;\
border: 1px solid #ddd;\
color: white;\
cursor: pointer;\
}\
</style>\
</head>\
<body>\
<h1> RSnapShot </h1>\
<fieldset>\
<form class=\"form-inline\" action=\"/Wifi_Data\">\
<h3> Timer </h3>\
Wait <input type=\"text\" name=\"Timer\" value=\"0\">s<br>\
<h3> Shots </h3>\
Take <input type=\"text\" name=\"Shots\" value=\"1\"> picture(s)<br>\
<h3>Shutter speed</h3>\
<input id=\"input\" type=\"range\" name=\"Shutter_speed\" min=\"0\" value=\"0\" max=\"46\" step=\"1\">\
<div id=\"output\"></div>\
<script language=\"JavaScript\">\
var values = [\"1/1000\", \"1/800\", \"1/640\", \"1/500\", \"1/400\", \"1/320\", \"1/250\", \"1/200\", \"1/160\", \"1/125\", \"1/100\", \"1/80\", \"1/60\", \"1/50\", \"1/40\", \"1/30\", \"1/25\", \"1/20\", \"1/15\", \"1/13\", \"1/10\", \"1/8\", \"1/6\", \"1/5\", \"1/4\", \"0\"3\", \"0\"4\", \"0\"5\", \"0\"6\", \"0\"8\", \"1\"\", \"1\"3\", \"1\"6\", \"2\"\", \"2\"5\", \"3\"2\", \"4\"\", \"5\"\", \"6\"\", \"8\"\", \"10\"\", \"13\"\", \"15\"\", \"20\"\", \"25\"\", \"30\"\", \"BULB\"];\
var input = document.getElementById('input'),\
output = document.getElementById('output');\
input.oninput = function(){\
output.innerHTML = values[this.value];\
};\
input.oninput();\
</script>\
<br>\
<br>\
<button type=\"submit\">Submit</button>\
</form>\
</fieldset>\
<div style=\"background-color:lightblue\">\
<h3>Instructions</h3>\
<p>If you need instructions, I don't think you should be using this in the first place.</p>\
<p>Tonix&reg (ft. Luigi)</p>\
</div>\
</body>\
</html>"

#endif