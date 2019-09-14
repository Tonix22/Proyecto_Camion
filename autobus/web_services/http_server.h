#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__
typedef void (*Flash_func_t)(char* VALUE);
void http_server_netconn_init(void);
void http_paser(char *buf);

#define HTMLCODE "<!DOCTYPE html>\
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
<style>\
alinear_precio {\
  margin-right: 150px;\
  margin-left: 7px;\
}\
</style>\
</head>\
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
<html>\
<body>\
<form>\
 <fieldset>\
  <legend>Configuration:</legend>\
  </form>\
  <form class=\"form-inline\" action=\"/Wifi_Data\">\
  <h3> Para conectarse a internet </h3>\
  <alinear_wifi>\
	  Wifi: <input type=\"text\" name=\"Wifi\"><br>\
  </alinear_wifi>\
  Password: <input type=\"Password\" name=\"Password\" ><br>\
  <h3> Informacion de ticket </h3>\
  <alinear_ruta>\
  RUTA: <input type=\"text\" name=\"RUTA\"><br>\
  </alinear_ruta>\
  UNIDAD: <input type=\"text\" name=\"UNIDAD\"><br>\
  <alinear_precio>\
  PRECIO: <input type=\"text\" name=\"PRECIO\"><br>\
  </alinear_precio>\
  <h3> Le enviaremos un reporte a este correo </h3>\
  <alinear_mail>\
  MAIL: <input type=\"text\" name=\"MAIL\"><br>\
  </alinear_mail>\
  <h3> Hora del correo </h3>\
  Hora:minuto AM/PM: <input type=\"time\" name=\"usr_time\"><br>\
  <br>\
  <br>\
  <button type=\"submit\">Submit</button>\
  </form>\
</fieldset>\
<div style=\"background-color:lightblue\">\
  <h3>Instrucciones</h3>\
  <p>Ponga la contrasena del wifi y contrasena de su router.</p>\
  <p>Llene los siguientes campos para programar la informacion de ticket.</p>\
  <p>Trate de no dejar campos vacios</p>\
  <p>Tonix&reg </p>\
</div>\
</body>\
</html>\
<html> \0"
#endif