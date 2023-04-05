#include "proxy.h"


/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

int 
main(int argc, char** argv)
{
    /* Check the input invalidation */
    if(argc != 2){
        fprintf(stderr, "usage: %s <port_number> \n", argv[0]);
        exit(EXIT_FAILURE);
    }
    char hostname[MAXLINE], port[MAXLINE];
    int listenfd, connfd;
    struct sockaddr_storage clientaddr;

    socklen_t client_len;

    listenfd = Open_listenfd(argv[1]);

    pthread_t tid;
    init_thread();
    /* Create assistent threads to help manage the thread pool*/
    Pthread_create(&tid, NULL, adjust_thread, NULL);
    setbuf(stdout, NULL);
    /* Do a infinite loop for listening the incoming resquest */
    while(1){
        client_len = sizeof(clientaddr);
        /* Get the connect fd */
        connfd = Accept(listenfd, (SA *)&clientaddr, &client_len);
        /* Convert a socket address structure to the corresponding host and service name string*/
        Getnameinfo((SA *)&clientaddr, client_len, hostname, MAXLINE, port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        insert_wrapper(connfd);
    }

    deinit_wrapper();
    return 0;
}

void 
modify_http_version(int fd, char *method, char *v){
    char current_version[] = "HTTP/1.0";
	if(strcasecmp(v, current_version) == 0){
        printf("version is corret, no need to change\n");
		return;
    }
    
    if((strlen(v) < 8) || strncmp(v, "HTTP", 4)){
		printf("error, wrong version format: %s\n", v);
        reply_client_error(fd, method, ERROR_CODE_NOT_IMPLEMENT, 
                            NOT_IMPLEMENT_SHORT_MSG, WRONG_VERSION_FORMAT);
	}
    memset(v, 0, strlen(v));
    strncpy(v, current_version, strlen(current_version));
}

void
parse_uri(char* url, char *uri, char *host, char *port){
	/* Remove http:// -- localhost:15213/home.html */
	char *no_prefix = &url[strlen(HTTP_PREFIX)];
	// /* :15213/home.html */
	char *colon = strstr(no_prefix, COLON);
	char *slash = strstr(no_prefix, SLASH);
	/* /home.html */
    strncpy(uri, slash, strlen(slash) + 1);
	/* localhost */
	strncpy(host, no_prefix, colon - no_prefix);
	/* 15213 */
	strncpy(port, colon + 1, slash - colon - 1);
}

void 
reply_client_error(int fd, char* cause, char* errnum, char *shortmsg, char* longmsg){
    /**
     * HTTP/1.0 501 Not Implemented
     * Content-type: text/html
     * 
     * <html><title>Tiny Error</title>
     * <body bgcolor=ffffff>
     * 501 Not Implemented
     * <p>Proxy does not support this method: POST</p>
     * <hr><em>The Tiny Web server</em>
     * </body></html>
     * 
     */
    char buf[MAXLINE];

    /* Print the HTTP response header */
    sprintf(buf,"HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf,"Content-type: text/html\r\n\r\n");
    Rio_writen(fd, buf, strlen(buf));

    /* Print the HTTP response body */
    sprintf(buf, "<html><title>Tiny Error</title>\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<body bgcolor=ffffff>\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "%s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<p>%s: %s</p>\r\n", longmsg, cause);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<hr><em>The Tiny Web server</em>\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "</body></html>\r\n");
    Rio_writen(fd, buf, strlen(buf));
}

int
change_request(rio_t *rp, char *clientRequest, char *uri, char *version, char* hostname){
    char buf[MAXLINE];
    int n; /* the bytes has read */
    
    int has_user_agent = 0, has_connection = 0, has_proxy_connection = 0, has_host = 0;

    /**
     * GET /home.html HTTP/1.0
     * Host: localhost
     * User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3
     * Connection: close
     * Proxy-Connection: close
     * 
     */
    sprintf(clientRequest, "GET %s %s\r\n", uri, version);
    n = Rio_readlineb(rp, buf, MAXLINE);
    printf("%s\n", buf);

    char *tmp;
    while((strcmp(buf, "\r\n") != 0) && (n != 0)){
        strcat(clientRequest, buf);
        if( (tmp = strstr(buf, "User-Agent:")) != NULL )
            has_user_agent = 1;
        else if((tmp = strstr(buf, "Proxy-Connection:")) != NULL)
            has_proxy_connection = 1;
        else if((tmp = strstr(buf, "Connection:")) != NULL)
            has_connection = 1;
        else if((tmp = strstr(buf, "Host:")) != NULL)
            has_host = 1;

        n = Rio_readlineb(rp, buf, MAXLINE);
    }
    if(n == 0)
        return 0;
    /* Change Request Header */
    if(has_host == 0){
        sprintf(buf, "Host: %s\r\n", hostname);
        strcat(clientRequest, buf);
    }
    /* append User-Agent */
    if(has_user_agent == 0){
        strcat(clientRequest, user_agent_hdr);
    }

    if(has_connection == 0){
        sprintf(buf, "Connection: close\r\n");
        strcat(clientRequest, buf);
    }

    if(has_proxy_connection == 0){
        sprintf(buf, "Proxy-Connection: close\r\n");
        strcat(clientRequest, buf);
    }

    strcat(clientRequest, "\r\n");
    
    return 1;
}

/**
 * @brief Foward request to the other server
 * 
 * @param connectfd 
 */
void 
process_request(int fd){

    char buf[MAXLINE], method[MAXLINE], url[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char hostname[MAXLINE], port[MAXLINE];
    char clientRequest[MAX_CACHE_SIZE];

    /* parse http request */
    rio_t rio_received, rio_forward;
    /* read request line and headers */
    Rio_readinitb(&rio_received, fd);
    if(!Rio_readlineb(&rio_received, buf, MAXLINE)){
        printf("read line failed\n");
        return ;
    }
    if(!strcmp(buf, "\r\n")){
        printf("bad request\n");
        reply_client_error(fd, method, ERROR_CODE_BAD_REQUEST, 
                            BAD_REQUEST, BAD_REQUEST);
        return;
    }
    printf("Receive request from the client: %s\n", buf);
    /**
     * @brief Read and parse the request line
     *  
     * HTTP Requests: method  URI        version 
     * For example:   GET    /index.html HTTP/1.1
     * 
     */
    sscanf(buf, "%s %s %s", method, url, version);
    
    printf("From client:\tmethod: %s, uri: %s, version: %s\n", method, url, version);

    /* Only supports for GET method */
    if(strcasecmp(method, "GET")){
        reply_client_error(fd, method, "501", 
                            "Not Implemented", "Tiny Does not implement this method");
        return ;
    }

    /* Replace http version the HTTP/1.0 */
    modify_http_version(fd, method, version);

    parse_uri(url, uri, hostname, port);

    printf("Proxy modified:\turl: %s, method: %s, version: %s, uri: %s, hostname: %s, port: %s\n", 
                    url, method, version, uri, hostname, port);
                    
    /* change the request header */
    int res = change_request(&rio_received, clientRequest, uri, version, hostname);
    if(res == 0)
        return ;

    printf("client request:\n%s", clientRequest);
    /** 
     * establish new connection with the server and
     * forward the request to the server 
     **/
    int clientfd = Open_clientfd(hostname, port);
    Rio_readinitb(&rio_forward, clientfd);
    Rio_writen(rio_forward.rio_fd, clientRequest, strlen(clientRequest));

    char responseBuf[MAXLINE];
    int n;
    while( (n = Rio_readnb(&rio_forward, responseBuf, MAXLINE)) != 0 ){
        Rio_writen(fd, responseBuf, n);
    }
    Close(clientfd);
}
