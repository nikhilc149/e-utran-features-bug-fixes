/*
 * Copyright (c) 2019 Sprint
 * Copyright (c) 2020 T-Mobile
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cp_app.h"
#include "ipc_api.h"

int
create_ipc_channel( void )
{
	/* STREAM - BiDirectional
	  DATAGRAM - uniDirectional */
	int sock ;

	/* Unix Socket Creation and Verification */
	sock = socket( AF_UNIX, SOCK_STREAM, 0);
	if ( sock == -1 ){
		fprintf(stderr,"%s: Unix socket creation failed. Error:%s\n",
				__func__, strerror(errno));

		return -1;
	}
	return sock;
}

int
connect_to_ipc_channel(int sock, struct sockaddr_un sock_addr, const char *path)
{
	int rc = 0;

	socklen_t  len = LENGTH;
	sock_addr.sun_family = AF_UNIX;

	chmod( path, 755 );

	strncpy(sock_addr.sun_path, path, sizeof(sock_addr.sun_path));

	rc = connect( sock, (struct sockaddr *) &sock_addr, len);
	if ( rc == -1) {
		fprintf(stderr, "%s: Could not connect to socket. Error: %s\n",
				__func__, strerror(errno));
		close_ipc_channel( sock );
	}

	return rc;
}

void
bind_ipc_channel(int sock, struct sockaddr_un sock_addr,const char *path)
{
	int rc = 0;
	/* Assign specific permission to path file read, write and executable */
	chmod( path, 755 );

	/* Assign Socket family and PATH */
	sock_addr.sun_family = AF_UNIX;
	strncpy(sock_addr.sun_path, path, strnlen(path,MAX_PATH_LEN));

	/* Remove the symbolic link of path names */
	unlink(path);
	/* Bind the new created socket to given PATH and verification */
	rc =  bind( sock, (struct sockaddr *) &sock_addr, LENGTH);
	if( rc != 0 ){
		close_ipc_channel(sock);
		printf("%s: Could not bind to socket. Error: %s\n",
				__func__, strerror(errno));

		/*Greacefull Exit*/
		exit(0);
	}
}

int
accept_from_ipc_channel(int sock, struct sockaddr_un sock_addr)
{
	int client_sock = 0;
	socklen_t len ;
	len = sizeof(sock_addr);

	while (1) {
		/* Accept incomming connection request receive on socket */
		client_sock = accept( sock, (struct sockaddr *) &sock_addr, &len);
		if (client_sock < 0){
			if (errno == EINTR)
				continue;

			close_ipc_channel(sock);
			printf("%s: Could not accept socket connection."
				"Error: %s\n", __func__,strerror(errno));

		} else {
			break;
		}
	}

	return client_sock;
}

void
listen_ipc_channel( int sock )
{
	/* Mark the socket as a passive socket to accept incomming connections */
	if( listen(sock, BACKLOG) == -1){
		close_ipc_channel(sock);
		printf("%s: Socket Listen failed error: %s\n",
				__func__, strerror(errno));

		/* Greacefull Exit */
		exit(0);
	}
}

void
get_peer_name(int sock, struct sockaddr_un sock_addr)
{
	socklen_t  len = LENGTH;
	if( getpeername( sock, (struct sockaddr *) &sock_addr, &len) == -1) {
		if(errno != EINTR)
		{
			fprintf(stderr, "%s: Socket getpeername failed error: %s\n",
					__func__, strerror(errno));
			close_ipc_channel(sock);
			/* Greacefull Exit */
			exit(0);
		}
	} else {
		fprintf(stderr, "CP: Gx_app client socket path %s...!!!\n",sock_addr.sun_path);
	}

}

int
recv_from_ipc_channel(int sock, char *buf)
{
	int bytes_recv = 0;
	bytes_recv = recv(sock, buf, BUFFSIZE, 0 ) ;
	if ( bytes_recv <= 0 ){
		if(errno != EINTR){
			fprintf(stderr, "%s: Socket recv failed error: %s\n",
					__func__, strerror(errno));
			close_ipc_channel(sock);
			/* Greacefull Exit */
			exit(0);
		}
	}

	return bytes_recv;
}

int
send_to_ipc_channel(int sock, uint8_t *buf, int len)
{
	int rc = 0;
	if ((rc = send(sock, buf, len, 0)) <= 0){
		if(errno != EINTR){
			fprintf(stderr, "%s: Socket send failed error: %s\n",
					__func__, strerror(errno));
			close_ipc_channel(sock);
			/* Greacefull Exit */
			exit(0);
		}
	}

	return rc;
}

void
close_ipc_channel(int sock)
{
	/* Closed unix socket */
	close(sock);
}

