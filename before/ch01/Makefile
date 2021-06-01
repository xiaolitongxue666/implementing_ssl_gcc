all: http webserver

OPTS = -g -Wall -Wno-pointer-sign

http: http.c base64.c base64.h
	gcc-9 -o http $(OPTS) http.c base64.c

webserver: webserver.c
	gcc-9 -o webserver $(OPTS) webserver.c

clean:
	rm -f http
	rm -f webserver
	rm -f core
	rm -f core.*
