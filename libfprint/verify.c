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
static void on_http_response(http_s *h);


struct fp_print_data *data;
int userId;

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
	sleep(1);
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

int verify(struct fp_dev *dev, struct fp_print_data *data)
{
	int r;

	do {
		struct fp_img *img = NULL;

		sleep(1);
		
		fprintf(stdout, "Please scan your fingerprint...\n");
		char msg[100];
		sprintf(msg, "{\"message\":\"Please scan your fingerprint...\",\"userId\":\"%d\"}", userId);
		sendViewMessage(msg, strlen(msg));
	
		r = fp_verify_finger_img(dev, data, &img);
		/*if (img) {
			fp_img_save_to_file(img, "verify.pgm");
			printf("Wrote scanned image to verify.pgm\n");
			fp_img_free(img);
		}*/
		if (r < 0) {
			fprintf(stderr, "Verification failed with error %d\n", r);
			sprintf(msg, "{\"message\":\"Verification failed with error %d\",\"userId\":\"%d\"}", r, userId);
			sendViewMessage(msg, strlen(msg));
			return r;
		}
		switch (r) {
		case FP_VERIFY_NO_MATCH:
			fprintf(stdout, "NO MATCH!\n");
			sprintf(msg, "{\"message\":\"NO MATCH!\",\"userId\":\"%d\"}", userId);
			sendViewMessage(msg, strlen(msg));
			return 0;
		case FP_VERIFY_MATCH:
			fprintf(stdout, "MATCH!\n");
			sprintf(msg, "{\"message\":\"MATCH!\",\"userId\":\"%d\"}", userId);
			sendViewMessage(msg, strlen(msg));
			return 0;
		case FP_VERIFY_RETRY:
			fprintf(stdout, "Scan didn't quite work. Please try again.\n");
			sprintf(msg, "{\"message\":\"Scan didn't quite work. Please try again\",\"userId\":\"%d\"}", userId);
			sendViewMessage(msg, strlen(msg));
			break;
		case FP_VERIFY_RETRY_TOO_SHORT:
			fprintf(stdout, "Swipe was too short, please try again.\n");
			sprintf(msg, "{\"message\":\"Swipe was too short, please try again\",\"userId\":\"%d\"}", userId);
			sendViewMessage(msg, strlen(msg));
			break;
		case FP_VERIFY_RETRY_CENTER_FINGER:
			fprintf(stdout, "Please center your finger on the sensor and try again.\n");
			sprintf(msg, "{\"message\":\"Please center your finger on the sensor and try again\",\"userId\":\"%d\"}", userId);
			sendViewMessage(msg, strlen(msg));
			break;
		case FP_VERIFY_RETRY_REMOVE_FINGER:
			fprintf(stdout, "Please remove finger from the sensor and try again.\n");
			sprintf(msg, "{\"message\":\"Please remove finger from the sensor and try again\",\"userId\":\"%d\"}", userId);
			sendViewMessage(msg, strlen(msg));
			break;
		}
	} while (1);
}

int startVerification()
{
	int r = 1;
	struct fp_dscv_dev *ddev;
	struct fp_dscv_dev **discovered_devs;
	struct fp_dev *dev;
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

	
	if (!data) {
		fprintf(stderr, "Failed to load fingerprint, error %d\n", r);
		fprintf(stderr, "Did you remember to enroll your finger first?\n");
		sprintf(msg, "{\"message\":\"Did you remember to enroll your finger first?\",\"userId\":\"%d\"}", userId);
		sendViewMessage(msg, strlen(msg));
		goto out_close;
	}
	
	verify(dev, data);

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
        fprintf(stdout, "Requested verification for user ID %d\n", userId);
  }else{
	  
	  fprintf(stderr,"Error parsing json message from webserver. No parameter data found.\n");
	  http_send_body(h, "ERROR", 5);
	  return;
  }
  
  fiobj_free(key);
  fiobj_free(obj);
  
  char msg[50];
  sprintf(msg, "Verification service received user ID = %d", userId);
  http_send_body(h, msg, strlen(msg));
  
  char* ip = getenv("WEBSERVER");
  char* port = getenv("WEBSERVER_PORT");
  char header[1024];
  
  sprintf(header, "%s:%s/getFingerprint?userId=%d", ip, port, userId);
  
  http_connect(header, NULL, .on_response = on_http_response);
  
  
}

static void on_http_response(http_s *h) {
	
  if(h->status_str == FIOBJ_INVALID){
	  //ignore the first empty response
	  http_finish(h);
	  return;
  }
  
  http_parse_body(h);
  FIOBJ obj = h->body;
  fio_str_info_s raw = fiobj_obj2cstr(obj);
  fprintf(stdout, "Fingerprint received - size = %d\n", raw.len);
  
  data = fp_print_data_from_data((unsigned char *)raw.data, raw.len);

  fiobj_free(obj);

  char msg[100];

  if(data){
    fprintf(stdout, "Verification service loaded the enrolled fingerprint - user ID = %d\n", userId);
    
    sprintf(msg, "Verification service loaded the enrolled fingerprint - user ID = %d", userId);
    sendViewMessage(msg, strlen(msg));
    
    http_send_body(h, msg, strlen(msg));

    startVerification();
    
    sprintf(msg, "{\"message\":\"DISCONNECT\",\"userId\":\"%d\"}", userId);
    sendViewMessage(msg, strlen(msg));
  
  }else {
    fprintf(stderr, "Fingerprint data conversion failed!\n");
    sprintf(msg, "{\"message\":\"Fingerprint data conversion failed. Could not load the user fingerprint\",\"userId\":\"%d\"}", userId);
    sendViewMessage(msg, strlen(msg));
    
    sprintf(msg, "{\"message\":\"DISCONNECT\",\"userId\":\"%d\"}", userId);
    sendViewMessage(msg, strlen(msg));
  }
  
}

int main() {
  
  //spring boot rest API IP and port
  char* ip = getenv("WEBSERVER");
  char* port = getenv("WEBSERVER_PORT");
  
  if(strlen(ip) == 0 || strlen(port) == 0){
    fprintf(stderr, "Environment variables WEBSERVER and WEBSERVER_PORT not set.\n");
    return 1;
  } else{
    fprintf(stdout, "WEBSERVER=%s, WEBSERVER_PORT=%s\n", ip, port);
  }

  if (http_listen("3000", NULL,
                  .on_request = on_http_request) == -1) {
    fprintf(stderr, "facil.io couldn't initialize HTTP service (already running?)\n");
    return 1;
  }
  
  fprintf(stdout, "Fingerprint Verification service started. Listening on port 3000...\n");
  
  fio_start(.threads = 1, .workers = 1);
  fio_cli_end();
  return 0;
}


