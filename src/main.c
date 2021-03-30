#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "http/cookie.h"
#include "http/http.h"
#include "dg/debug.h"


struct {
	char *ext;
	char *filetype;
} extensions [] = {
	{"gif", "image/gif" },
	{"jpg", "image/jpg" },
	{"jpeg","image/jpeg"},
	{"png", "image/png" },
	{"ico", "image/ico" },
	{"zip", "image/zip" },
	{"gz",  "image/gz"  },
	{"tar", "image/tar" },
	{"htm", "text/html" },
	{"html","text/html" },
	{0,0} };

void process_web_request(int fd)
{
    dg_log("Processing a new request");

    http_error_t err;
    http_req_t* req = http_read_request(fd, &err);
    if (req == NULL)
        dg_log("The request could not be read");

    http_res_t* res;
    if (req->method != GET) {
        res = http_new_response("HTTP/1.1", 400, "Bad Request",
                               1, http_new_headers(1,
                                    "Connection", "close"),
                                "");
        http_write_response(fd, res, &err);
    } else {
        char* url = "/index.html";
        if (strcmp(req->url, "/") != 0) {
            url = req->url;
        }

        if (access(url + 1, F_OK | R_OK) != 0) {
            dg_log("Request tried to access a non-existent file");
            res = http_new_response("HTTP/1.1", 404, "Not Found",
                                    1, http_new_headers(1,
                                        "Connection", "close"),
                                    "");
            http_write_response(fd, res, &err);
        } else {
            dg_log("Request was OK");
            http_cookie_t* cookie = http_get_cookie(req, "cookie_counter");
            if (cookie == NULL || atoi(cookie->value) < 10) {
                char* new_counter = malloc(3);
                if (new_counter == NULL) {
                    dg_err("Memory error");
                    exit(1);
                }
                sprintf(new_counter, "%d", (cookie == NULL) ? 1 : (atoi(cookie->value) + 1));
                http_cookie_t* new_cookie = http_new_cookie(
                        "cookie_counter", new_counter,
                        1, 2*60);
                char* new_cookie_str = http_cookie_to_string(new_cookie);
                res = http_new_response("HTTP/1.1", 200, "OK",
                        2, http_new_headers(2,
                            "Connection", "close",
                            "Set-Cookie", new_cookie_str),
                        "");

                free(new_cookie_str);
                http_drop_cookie(new_cookie);
                http_drop_cookie(cookie);
                int file_fd = open(url + 1, O_RDONLY, S_IRUSR);
                if (file_fd == -1) {
                    dg_err("Could not read file");
                    close(fd);
                    exit(1);
                }

                http_write_response(fd, res, &err);
                ssize_t bytes_sent = 0;
                off_t offset = 0;
                do {
                    bytes_sent = sendfile(fd, file_fd, &offset, 8000);
                } while (bytes_sent > 0);
                close(file_fd);
            } else {
                res = http_new_response("HTTP/1.1", 200, "OK",
                        1, http_new_headers(1,
                            "Connection", "close"),
                        "");
                int file_fd = open("error.html", O_RDONLY, S_IRUSR);
                if (file_fd == -1) {
                    dg_err("Could not read file");
                    close(fd);
                    exit(1);
                }

                http_write_response(fd, res, &err);
                ssize_t bytes_sent = 0;
                off_t offset = 0;
                do {
                    bytes_sent = sendfile(fd, file_fd, &offset, 8000);
                } while (bytes_sent > 0);
                close(file_fd);
            }

        }
    }

    http_drop_request(req);
    http_drop_response(res);
	//	Evaluar el tipo de fichero que se está solicitando, y actuar en
	//	consecuencia devolviendolo si se soporta u devolviendo el error correspondiente en otro caso
    dg_log("Finished handling request");
}

int main(int argc, char **argv) {
	int i, port, pid, listenfd, socketfd;
	socklen_t length;
	static struct sockaddr_in cli_addr;		// static = Inicializado con ceros
	static struct sockaddr_in serv_addr;	// static = Inicializado con ceros
	
	//  Argumentos que se esperan:
	//
	//	argv[1]
	//	En el primer argumento del programa se espera el puerto en el que el servidor escuchara
	//
	//  argv[2]
	//  En el segundo argumento del programa se espera el directorio en el que se encuentran los ficheros del servidor
	//
	//  Verficiar que los argumentos que se pasan al iniciar el programa son los esperados
	//

	//
	//  Verficiar que el directorio escogido es apto. Que no es un directorio del sistema y que se tienen
	//  permisos para ser usado
	//
    

	if(chdir(argv[2]) == -1){ 
		(void)printf("ERROR: No se puede cambiar de directorio %s\n",argv[2]);
		exit(4);
	}
    dg_open("webserver.log");
	// Hacemos que el proceso sea un demonio sin hijos zombies
	if(fork() != 0)
		return 0; // El proceso padre devuelve un OK al shell

	(void)signal(SIGCHLD, SIG_IGN); // Ignoramos a los hijos
	(void)signal(SIGHUP, SIG_IGN); // Ignoramos cuelgues
	
    dg_log("Web server starting");
	
	/* setup the network socket */
	if((listenfd = socket(AF_INET, SOCK_STREAM,0)) <0) {
        dg_err("Syscall socket");
        exit(1);
    }
	
	port = atoi(argv[1]);
	
	if (port < 0 || port >60000) {
        dg_err("Invalid port");
        exit(1);
    } 
	
	/*Se crea una estructura para la información IP y puerto donde escucha el servidor*/
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); /*Escucha en cualquier IP disponible*/
	serv_addr.sin_port = htons(port); /*... en el puerto port especificado como parámetro*/
	
	if(bind(listenfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr)) <0) {
        dg_err("Syscall bind");
        exit(1);
    }
	
	if( listen(listenfd,64) <0) {
        dg_err("Syscall listen");
        exit(1);
    }
	
	while (1) {
		length = sizeof(cli_addr);
		if((socketfd = accept(listenfd, (struct sockaddr *)&cli_addr, &length)) < 0) {
            dg_err("Syscall accept");
            exit(1);
        }
		if((pid = fork()) < 0) {
            dg_err("Syscall fork");
		}
		else {
			if(pid == 0) { 	// Proceso hijo
                (void)close(listenfd);

                fd_set read_fd;
                struct timeval tv;

                do  {
                    process_web_request(socketfd); // El hijo termina tras llamar a esta función

                    FD_ZERO(&read_fd);
                    FD_SET(socketfd, &read_fd);

                    tv.tv_sec  = 5;
                    tv.tv_usec = 0;
                } while (select(1, &read_fd, NULL, NULL, &tv));

                (void)close(socketfd);
                (void)exit(0);
			} else { 	// Proceso padre
				(void)close(socketfd);
			}
		}
	}
}
