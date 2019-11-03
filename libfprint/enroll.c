#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include "fio_cli.h"
#include "http.h"

#include <fprint.h>

static void on_http_request(http_s *h);

int userId;
char deviceName[1024];

//TODO: implement post with facil.io
int socket_connect(char *host, in_port_t port){
	//fprintf(stdout, "Connecting to IP |%s| Port |%d|\n", host, port);
	struct hostent *hp;
	struct sockaddr_in addr;
	int on = 1, sock;     

	if((hp = gethostbyname(host)) == NULL){
		fprintf(stderr,"Error gethostbyname\n");
		return 1;
	}
	bcopy(hp->h_addr, &addr.sin_addr, hp->h_length);
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char *)&on, sizeof(int));

	if(sock == -1){
		fprintf(stderr,"Error setsockopt.\n");
		return 1;
	}
	
	if(connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1){
		fprintf(stdout, "Error socket connect\n");
		return 1;

	}
	
	return sock;	
}

void sendViewMessage(char* msg, int msgLen){
	char header[1024];
	char* ip = getenv("WEBSERVER");
	char* port = getenv("WEBSERVER_PORT");
	int sock = socket_connect(ip, atoi(port)); 
					
	sprintf(header, "POST /libfprintMessages HTTP/1.1\r\n"
					"Host: %s\r\n"
					"Content-Type: application/json\r\n"
					"Content-Length: %d\r\n"
					"Connection: close\r\n"
					"\r\n"
					"%s\r\n"
					"\r\n", ip, msgLen, msg);
			
	write(sock, header, strlen(header));

	shutdown(sock, SHUT_RDWR); 
	close(sock); 
}

struct fp_dscv_dev *discover_device(struct fp_dscv_dev **discovered_devs)
{
	struct fp_dscv_dev *ddev = discovered_devs[0];
	struct fp_driver *drv;
	if (!ddev)
		return NULL;
	
	drv = fp_dscv_dev_get_driver(ddev);
	
	fprintf(stdout, "Found device claimed by %s driver\n", fp_driver_get_full_name(drv));
	
	char msg[100];
	sprintf(msg, "{\"message\":\"Found device claimed by %s driver\",\"userId\":\"%d\"}", fp_driver_get_full_name(drv), userId);
	sendViewMessage(msg, strlen(msg));
	
	return ddev;
}

struct fp_print_data *enroll(struct fp_dev *dev) {
	struct fp_print_data *enrolled_print = NULL;
	int r;

	fprintf(stdout,"Scan your finger %d times to enroll.\n", fp_dev_get_nr_enroll_stages(dev));
	
	char msg[100];
	sprintf(msg, "{\"message\":\"Scan your finger %d times to enroll...\",\"userId\":\"%d\"}", fp_dev_get_nr_enroll_stages(dev), userId);
	sendViewMessage(msg, strlen(msg));

	do {
		struct fp_img *img = NULL;
		
		fprintf(stdout, "Waiting for scan...");

		r = fp_enroll_finger_img(dev, &enrolled_print, &img);
		/*if (img) {
			fp_img_save_to_file(img, "enrolled.pgm");
			printf("Wrote scanned image to enrolled.pgm\n");
			fp_img_free(img);
		}*/
		if (r < 0) {
			fprintf(stderr,"Enrollment failed with error %d\n", r);
			sprintf(msg, "{\"message\":\"Enrollment failed with error %d\",\"userId\":\"%d\"}", r, userId);
			sendViewMessage(msg, strlen(msg));
			return NULL;
		}

		switch (r) {
		case FP_ENROLL_COMPLETE:
			fprintf(stdout,"Enrollment completed!\n");
			sprintf(msg, "{\"message\":\"Enrollment completed!\",\"userId\":\"%d\"}", userId);
			sendViewMessage(msg, strlen(msg));
			break;
		case FP_ENROLL_FAIL:
			fprintf(stdout,"Enroll failed, something wen't wrong\n");
			sprintf(msg, "{\"message\":\"Enroll failed, something wen't wrong\",\"userId\":\"%d\"}", userId);
			sendViewMessage(msg, strlen(msg));
			return NULL;
		case FP_ENROLL_PASS:
			fprintf(stdout,"Enroll stage passed.\n");
			sprintf(msg, "{\"message\":\"Enroll stage passed :)\",\"userId\":\"%d\"}", userId);
			sendViewMessage(msg, strlen(msg));
			break;
		case FP_ENROLL_RETRY:
			fprintf(stdout,"Didn't quite catch that. Please scan again\n");
			sprintf(msg, "{\"message\":\"Didn't quite catch that. Please scan again\",\"userId\":\"%d\"}", userId);
			sendViewMessage(msg, strlen(msg));
			break;
		case FP_ENROLL_RETRY_TOO_SHORT:
			fprintf(stdout,"Your swipe was too short, please try again.\n");
			sprintf(msg, "{\"message\":\"Your swipe was too short, please try again\",\"userId\":\"%d\"}", userId);
			sendViewMessage(msg, strlen(msg));
			break;
		case FP_ENROLL_RETRY_CENTER_FINGER:
			fprintf(stdout,"Didn't catch that, please center your finger on the sensor and try again\n");
			sprintf(msg, "{\"message\":\"Didn't catch that, please center your finger on the sensor and try again\",\"userId\":\"%d\"}", userId);
			sendViewMessage(msg, strlen(msg));
			break;
		case FP_ENROLL_RETRY_REMOVE_FINGER:
			fprintf(stdout,"Scan failed, please remove your finger and then try again\n");
			sprintf(msg, "{\"message\":\"Scan failed, please remove your finger and then try again\",\"userId\":\"%d\"}", userId);
			sendViewMessage(msg, strlen(msg));
			break;
		}
	} while (r != FP_ENROLL_COMPLETE);

	if (!enrolled_print) {
		fprintf(stderr, "Enroll complete but no print?\n");
		return NULL;
	}
			
	return enrolled_print;
}



int startEnroll(int userId)
{
	int r = 1;
	struct fp_dscv_dev *ddev;
	struct fp_dscv_dev **discovered_devs;
	struct fp_dev *dev;
	struct fp_print_data *data;
	int fpDataLen;
	unsigned char *fpBuffer;
	int sock;
	char header[1024];
	char *packet;
	char* ip = getenv("WEBSERVER");
	char* port = getenv("WEBSERVER_PORT");
	char msg[100];
	
	//setenv ("G_MESSAGES_DEBUG", "all", 0);
	//setenv ("LIBUSB_DEBUG", "3", 0);

	r = fp_init();
	if (r < 0) {
		fprintf(stderr, "Failed to initialize libfprint\n");
		sprintf(msg, "{\"message\":\"Failed to initialize libfprint\",\"userId\":\"%d\"}", userId);
		sendViewMessage(msg, strlen(msg));
		return 1;
	}

	discovered_devs = fp_discover_devs();
	if (!discovered_devs) {
		fprintf(stderr, "Could not discover devices\n");
		sprintf(msg, "{\"message\":\"Could not discover devices\",\"userId\":\"%d\"}", userId);
		sendViewMessage(msg, strlen(msg));
		goto out;
	}

	ddev = discover_device(discovered_devs);
	if (!ddev) {
		fprintf(stderr, "No devices detected.\n");
		sprintf(msg, "{\"message\":\"No devices detected\",\"userId\":\"%d\"}", userId);
		sendViewMessage(msg, strlen(msg));
		goto out;
	}

	dev = fp_dev_open(ddev);
	fp_dscv_devs_free(discovered_devs);
	if (!dev) {
		fprintf(stderr, "Could not open device.\n");
		sprintf(msg, "{\"message\":\"Could not open device\",\"userId\":\"%d\"}", userId);
		sendViewMessage(msg, strlen(msg));
		goto out;
	}

	fprintf(stdout,"Opened device. Can start the enrollment...\n");
	
	data = enroll(dev);
	if (!data)
		goto out_close;
		
	fpDataLen = fp_print_data_get_data(data, &fpBuffer);
	
	if (r < 0){
		fprintf(stderr, "fp_print_data_get_data failed, code %d\n", r);
		sprintf(msg, "{\"message\":\"fp_print_data_get_data failed, code %d\",\"userId\":\"%d\"}", r, userId);
		sendViewMessage(msg, strlen(msg));
	}
		
	fprintf(stderr, "Fingerprint Data size = %d\n", fpDataLen);
	
	sock = socket_connect(ip, atoi(port)); 
					
	sprintf(header, "POST /saveFingerprint HTTP/1.1\r\n"
					"Host: %s\r\n"
					"Content-Type: application/octet-stream\r\n"
					"Content-Length: %d\r\n"
					"Connection: close\r\n"
					"user-id: %d\r\n"
					"\r\n", ip, fpDataLen, userId);
	
	int headerLen = strlen(header);
	int packLen = headerLen + fpDataLen;
	
	packet = calloc(1, packLen + 1);
	memcpy(packet, header, headerLen);
	memcpy(packet + headerLen, fpBuffer, fpDataLen);
			
	write(sock, packet, packLen);

	shutdown(sock, SHUT_RDWR); 
	close(sock); 
	
	sprintf(msg, "{\"message\":\"Fingerprint data sent to the database. Size=%d\",\"userId\":\"%d\"}", fpDataLen, userId);
	sendViewMessage(msg, strlen(msg));
	
	//to save in a binary file...
	/*FILE *file;
	file = fopen("fingerprint.txt", "wb");
	
	if(file == NULL)
		printf("Error opening file");
		
	fwrite(postResponse, sizeof(char), packLen, file);
	fclose(file);*/

	fp_print_data_free(data);
out_close:
	fp_dev_close(dev);
out:
	fp_exit();
	return 0;
}

static void on_http_request(http_s *h) {
  
  http_parse_body(h);
  FIOBJ obj = h->body;
  fio_str_info_s raw = fiobj_obj2cstr(obj);
  size_t consumed = fiobj_json2obj(&obj, raw.data, 1024);

  if (!consumed || !obj) {
    fprintf(stdout, "ERROR, couldn't parse data to start the enrollment.\n");
  }
  
  FIOBJ key = fiobj_str_new("userId", 6);
  fio_str_info_s param;
  if (FIOBJ_TYPE_IS(obj, FIOBJ_T_HASH) // make sure the JSON object is a Hash
      && fiobj_hash_get(obj, key)) {   // test for the existence of the key
        param = fiobj_obj2cstr(fiobj_hash_get(obj, key));
        userId = atoi(param.data);
        fprintf(stdout, "Requested enrollment for user ID %d\n", userId);
  }else{
	  fprintf(stderr,"Error parsing json message from webserver. No parameter data found.\n");
	  http_send_body(h, "ERROR", 5);
	  return;
	}
	
  fiobj_free(key);
  
  char msg[50];
  sprintf(msg, "Enrollment service received user ID = %d", userId);
  http_send_body(h, msg, strlen(msg));
  
  startEnroll(userId);
  
  
  sprintf(msg, "{\"message\":\"DISCONNECT\",\"userId\":\"%d\"}", userId);
  sendViewMessage(msg, strlen(msg));
}




int main() {
	
  //spring boot rest API IP and port
  char* ip = getenv("WEBSERVER");
  char* port = getenv("WEBSERVER_PORT");
  
  if(ip == NULL || port == NULL){
    fprintf(stderr, "Environment variables WEBSERVER and WEBSERVER_PORT are not set.\n");
    return 1;
  } else{
    fprintf(stdout, "Remote server configuration: WEBSERVER=%s, WEBSERVER_PORT=%s\n", ip, port);
  }

  if (http_listen("3000", NULL,
                  .on_request = on_http_request) == -1) {
    fprintf(stderr, "facil.io couldn't initialize HTTP service (already running?)\n");
    return 1;
  }
  
   fprintf(stdout, "Fingerprint Enrollment service started. Listening on port 3000...\n");
  
  
  fio_start(.threads = 1, .workers = 1);
  fio_cli_end();
  return 0;
}


