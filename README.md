# Nginx + Tcl

Nginx Module to Add The Power of Tcl


```nginx
location /webtcl-content {
  webtcl header "X-WebTcl" "Nginx + Tcl";

  webtcl echo "request = $request";

  webtcl puts "tcl_version = $tcl_version";
  
  webtcl variable nginx_uri $uri;
}
```
