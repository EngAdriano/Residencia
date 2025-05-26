#pragma once

const char *html_page =
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html\r\n\r\n"
"<!DOCTYPE html>\n"
"<html>\n"
"<head><title>GPIO 5 Monitor</title></head>\n"
"<body>\n"
"<h1>Status do Botao no GPIO 5</h1>\n"
"<p id='status'>Carregando...</p>\n"
"<script>\n"
"setInterval(() => {\n"
" fetch('/status')\n"
"  .then(res => res.text())\n"
"  .then(txt => document.getElementById('status').innerText = txt);\n"
"}, 1000);\n"
"</script>\n"
"</body>\n"
"</html>\n";
