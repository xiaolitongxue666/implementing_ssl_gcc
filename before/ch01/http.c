#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "base64.h"


#define HTTP_PORT 80

int parse_url( char *uri, char **host, char** path )
{
    char *pos;

    pos = strstr(uri, "//");

    if( !pos )
    {
        return -1;
    }

    *host = pos+2;

    pos = strchr( *host, '/' );

    if(!pos)
    {
        *path = NULL;
    }
    else
    {
        *pos = '\0';
        *path = pos +1;
    }

    return 0;

}


#define MAX_GET_COMMAND 255
int http_get(int connection,
        const char *path,
        const char *host,
        const char *proxy_host,
        const char *proxy_user,
        const char *proxy_passwd)
{
    static char get_command [ MAX_GET_COMMAND ];

    if(proxy_host)
    {
        sprintf(get_command, "GET http://%s/%s HTTP/1.1\r\n", host, path);
    }else{
        sprintf(get_command, "GET /%s HTTP/1.1\r\n", path);
    }

    if( send(connection, get_command, strlen(get_command), 0)  == -1 )
    {
        return -1;
    }

    sprintf(get_command, "Host: %s\r\n", host);
    if(send(connection, get_command, strlen(get_command), 0) == -1)
    {
        return -1;
    }

    if(proxy_user)
    {
        int credentials_len = strlen(proxy_user) + strlen(proxy_passwd) + 1;
        char *proxy_credentials = malloc(credentials_len);
        char *auth_string = malloc (((credentials_len*4)/3)+1);
        sprintf(proxy_credentials, "%s:%s", proxy_user, proxy_passwd);
        base64_encode(proxy_credentials, credentials_len, auth_string);
        sprintf(get_command, "Proxy-Authorization: BASIC%s\r\n", auth_string);
        if(send(connection, get_command, strlen(get_command), 0) == -1)
        {
            free(proxy_credentials);
            free(auth_string);
            return -1;
        }
        free(proxy_credentials);
        free(auth_string);
    }


    sprintf(get_command, "Connection: closr\r\n\r\n");
    if(send(connection, get_command, strlen(get_command), 0) == -1)
    {
        return -1;
    }

    return 0;
}


#define BUFFER_SIZE 255
void display_result(int connection)
{
    int received = 0;

    static char recv_buf[BUFFER_SIZE + 1];

    while((received = recv( connection, recv_buf, BUFFER_SIZE , 0)) > 0)
    {
        recv_buf[received] = '\0';
        printf("%s" , recv_buf);
    }
    printf("\n");
}

// proxy_spec is 'http://[username:password@]hostname[:port]/'
int parse_proxy_param(char *proxy_spec, char **proxy_host, int *proxy_port, char **proxy_user, char **proxy_passwd)
{
    char *login_sep, *colon_sep, *trailer_sep;
    if(!strncmp("http://", proxy_spec, 7))
    {
        proxy_spec += 7;//'http://' length is 7
    }

    // proxy_spec is '[username:password@]hostname[:port]'
    // Find '@'
    login_sep = strchr(proxy_spec, '@');
    // Judge if have '@' for proxy paramters
    if(login_sep)
    {
        colon_sep = strchr(proxy_spec, ':');
        // Find ':'
        //colon_sep is ':password@]hostname[:port]'
        if(!colon_sep || (colon_sep > login_sep))
        {
            fprintf(stderr, "Expected password in '%s'\n", proxy_spec);
            return 0;
        }

        *colon_sep = '\0';//Finded ':' set to '\0'
        *proxy_user = proxy_spec;//Return string to point parameter 'username'
        *login_sep = '\0';
        *proxy_passwd = colon_sep + 1;
        proxy_spec = login_sep + 1;
    }

    //proxy_spec is 'hostname[:port]/'
    trailer_sep = strchr(proxy_spec, '/');
    if(trailer_sep)
    {
        *trailer_sep = '\0';
    }

    colon_sep = strchr( proxy_spec, ':' );
    if(colon_sep)
    {
        *colon_sep = '\0';
        *proxy_host = proxy_spec;
        *proxy_port = atoi(colon_sep+ 1);
        if(*proxy_port == 0 )
        {
            return 0;
        }
    }
    else {
        *proxy_port = HTTP_PORT;
        *proxy_host = proxy_spec;
    }
    return 1;
}


////////////////////////main
int main(int argc, char *argv[])
{
    int client_connection = 0;
    char *proxy_host, *proxy_user, *proxy_password;
    int proxy_port;
    char *host, *path;
    struct hostent *host_name;
    struct sockaddr_in host_address;
    int ind;

    if (argc <2 ){
        fprintf(stderr, "Usage: %s:[-p http://[username:password@]proxy-host:proxy-port]<URL>\n", argv[0]);
        return 1;
    }

    proxy_host = proxy_user = proxy_password = host = path = NULL;
    ind = 1;

    // ./http -p http://[username:password@]hostname[:port]/
    // http://[username:password@]hostname[:port]/
    // ind = 1
    // argv[1] is '-p'

    if(!strcmp("-p", argv[ind]))
    {
        // Have proxy parameter
        // argv[++i] is 'http://[username:password@]hostname[:port]/'
        if(!parse_proxy_param(argv[++ind], &proxy_host, &proxy_port, &proxy_user, &proxy_password))
        {
            fprintf(stderr, "Error - malformed proxy parameter '%s'.\n" , argv[2]);
            return 2;
        }
        ind++;
    }


    if(parse_url(argv[1], &host, &path) == -1){
        fprintf(stderr, "Error - malformed URL %s .\n" , argv[1]);
        return 1;
    }

    if( proxy_host ){
        printf("Connecting to proxy host '%s'\n", proxy_host);

        host_name = gethostbyname(proxy_host);
    } else {

        printf("Connecting to host '%s'\n",host);

        host_name = gethostbyname(host);
    }

    if( !host_name )
    {
        perror("Error in name resolution");
        return 3;
    }

    host_address.sin_family = AF_INET;
    host_address.sin_port = htons(proxy_host ? proxy_port : HTTP_PORT);
    memcpy( &host_address.sin_addr, host_name->h_addr_list[0] ,
        sizeof(struct in_addr));

    if( connect( client_connection, (struct sockaddr *) &host_address, sizeof(host_address) ) == -1 )
    {
        perror("Unable to connect to host");
        return 4;
    }

    printf("Retrieving document: '%s'\n\r", path);

    http_get(client_connection, path, host, proxy_host, proxy_user, proxy_password);

    display_result(client_connection);

    printf("Shutting down .\n\r");

    if(close(client_connection) == -1){
        perror("Error closing client connection");
        return 5;
    }

    return 0;

}
