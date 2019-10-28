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

struct fp_dscv_dev *discover_device(struct fp_dscv_dev **discovered_devs)
{
	struct fp_dscv_dev *ddev = discovered_devs[0];
	struct fp_driver *drv;
	if (!ddev)
		return NULL;
	
	drv = fp_dscv_dev_get_driver(ddev);
	printf("Found device claimed by %s driver\n", fp_driver_get_full_name(drv));
	return ddev;
}

struct fp_print_data *enroll(struct fp_dev *dev) {
	struct fp_print_data *enrolled_print = NULL;
	int r;

	printf("You will need to successfully scan your finger %d times to "
		"complete the process.\n", fp_dev_get_nr_enroll_stages(dev));

	do {
		struct fp_img *img = NULL;

		printf("\nScan your finger now.\n");

		r = fp_enroll_finger_img(dev, &enrolled_print, &img);
		if (img) {
			fp_img_save_to_file(img, "enrolled.pgm");
			printf("Wrote scanned image to enrolled.pgm\n");
			fp_img_free(img);
		}
		if (r < 0) {
			printf("Enroll failed with error %d\n", r);
			return NULL;
		}

		switch (r) {
		case FP_ENROLL_COMPLETE:
			printf("Enroll complete!\n");
			break;
		case FP_ENROLL_FAIL:
			printf("Enroll failed, something wen't wrong :(\n");
			return NULL;
		case FP_ENROLL_PASS:
			printf("Enroll stage passed. Yay!\n");
			break;
		case FP_ENROLL_RETRY:
			printf("Didn't quite catch that. Please try again.\n");
			break;
		case FP_ENROLL_RETRY_TOO_SHORT:
			printf("Your swipe was too short, please try again.\n");
			break;
		case FP_ENROLL_RETRY_CENTER_FINGER:
			printf("Didn't catch that, please center your finger on the "
				"sensor and try again.\n");
			break;
		case FP_ENROLL_RETRY_REMOVE_FINGER:
			printf("Scan failed, please remove your finger and then try "
				"again.\n");
			break;
		}
	} while (r != FP_ENROLL_COMPLETE);

	if (!enrolled_print) {
		fprintf(stderr, "Enroll complete but no print?\n");
		return NULL;
	}

	printf("Enrollment completed!\n\n");
	return enrolled_print;
}

int socket_connect(char *host, in_port_t port){
	fprintf(stderr, "IP |%s|\nPort |%d|\n", host, port);
	struct hostent *hp;
	struct sockaddr_in addr;
	int on = 1, sock;     

	if((hp = gethostbyname(host)) == NULL){
		herror("gethostbyname");
		exit(1);
	}
	bcopy(hp->h_addr, &addr.sin_addr, hp->h_length);
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char *)&on, sizeof(int));

	if(sock == -1){
		perror("setsockopt");
		exit(1);
	}
	
	if(connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1){
		perror("connect");
		exit(1);

	}
	return sock;	
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
	unsigned char postResponse[1024];
	int sock;
	char header[1024];
	char *packet;
	char* ip = getenv("WEBSERVER");
	char* port = getenv("WEBSERVER_PORT");
	
	//setenv ("G_MESSAGES_DEBUG", "all", 0);
	//setenv ("LIBUSB_DEBUG", "3", 0);

	r = fp_init();
	if (r < 0) {
		fprintf(stderr, "Failed to initialize libfprint\n");
		exit(1);
	}

	discovered_devs = fp_discover_devs();
	if (!discovered_devs) {
		fprintf(stderr, "Could not discover devices\n");
		goto out;
	}

	ddev = discover_device(discovered_devs);
	if (!ddev) {
		fprintf(stderr, "No devices detected.\n");
		goto out;
	}

	dev = fp_dev_open(ddev);
	fp_dscv_devs_free(discovered_devs);
	if (!dev) {
		fprintf(stderr, "Could not open device.\n");
		goto out;
	}

	printf("Opened device. It's now time to enroll your finger.\n\n");
	data = enroll(dev);
	if (!data)
		goto out_close;
		
	fpDataLen = fp_print_data_get_data(data, &fpBuffer);
	
	if (r < 0)
		fprintf(stderr, "Data save failed, code %d\n", r);
		
	fprintf(stderr, "Fingerprint Data size = %d\n", fpDataLen);
	
	sock = socket_connect(ip, atoi(port)); 
					
	sprintf(header, "POST /save HTTP/1.1\r\n"
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
	
	fprintf(stderr, "Sending buffer:\n %s\n\n", packet);
			
	write(sock, packet, packLen);
	
	while(read(sock, postResponse, packLen - 1) != 0){
		fprintf(stderr, "Response:\n%s\n\n", postResponse);
		bzero(postResponse, packLen);
	}

	shutdown(sock, SHUT_RDWR); 
	close(sock); 
	
	/*FILE *file;
	file = fopen("postResponse.txt", "wb");
	
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
    perror("\nERROR, couldn't parse data.\n");
  }
  
  FIOBJ key = fiobj_str_new("parameter", 9);
  fio_str_info_s value;
  if (FIOBJ_TYPE_IS(obj, FIOBJ_T_HASH) // make sure the JSON object is a Hash
      && fiobj_hash_get(obj, key)) {   // test for the existence of the key
        value = fiobj_obj2cstr(fiobj_hash_get(obj, key));
        fprintf(stderr, "user id=%s\n", value.data);
  }else{
	  perror("Invalid parse data.");
	}
  fiobj_free(key);
  
  http_send_body(h, "OK", 2);
  
  startEnroll(atoi(value.data));
}

int main() {

  if (http_listen("3000", NULL,
                  .on_request = on_http_request) == -1) {
    perror(
        "ERROR: facil.io couldn't initialize HTTP service (already running?)");
    exit(1);
  }
  
  fio_start(.threads = 1, .workers = 1);
  fio_cli_end();
  return 0;
}


