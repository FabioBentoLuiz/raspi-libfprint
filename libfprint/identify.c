#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "fprint.h"
#include "fio_cli.h"
#include "http.h"

static void on_http_request(http_s *h);
static void on_http_response(http_s *h);


struct fp_print_data **data;
int userId;

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

int verify(struct fp_dev *dev, struct fp_print_data **data)
{
	int r;

	do {
	        size_t matchOffset[50000];
		printf("\nScan your finger now.\n");
		r = fp_identify_finger(dev, data, matchOffset);
		
		
		if (r < 0) {
			printf("verification failed with error %d :(\n", r);
			return r;
		}
		
		switch (r) {
		case FP_VERIFY_NO_MATCH:
			printf("NO MATCH!\n");
			return 0;
		case FP_VERIFY_MATCH:
			printf("MATCH!\n");
			return 0;
		}
	} while (1);
}

int startIdentification()
{
	int r = 1;
	struct fp_dscv_dev *ddev;
	struct fp_dscv_dev **discovered_devs;
	struct fp_dev *dev;

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
	
	int hasIdentification = fp_dev_supports_identification(dev);
	if(!hasIdentification) {
	  fprintf(stderr, "Your fingerprint device doesn't support identification.\n");
	  goto out_close;
	}

	printf("Opened device. Loading previously enrolled right index finger "
		"data...\n");
		
	if (!data) {
		fprintf(stderr, "Failed to load fingerprint, error %d\n", r);
		fprintf(stderr, "Did you remember to enroll your right index finger "
			"first?\n");
		goto out_close;
	}

	printf("Print loaded. Time to verify!\n");
	
	verify(dev, data);

	fp_print_data_free(data[0]);
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
  char header[1024];

  if (!consumed || !obj) {
    perror("ERROR, couldn't parse data.\n");
  }
  
  FIOBJ key = fiobj_str_new("parameter", 9);
  fio_str_info_s param;
  if (FIOBJ_TYPE_IS(obj, FIOBJ_T_HASH) // make sure the JSON object is a Hash
      && fiobj_hash_get(obj, key)) {   // test for the existence of the key
        param = fiobj_obj2cstr(fiobj_hash_get(obj, key));
        userId = atoi(param.data);
        fprintf(stdout, "Requested enrollment for user ID %d\n", userId);
  }else{
      fprintf(stderr,"Error parsing json message from webserver. No parameter data found.\n");
      http_send_body(h, "ERROR", 2);
      return;
  }
  
  fiobj_free(key);
  
  http_send_body(h, "OK", 2);
  
  
  char* ip = getenv("WEBSERVER");
  char* port = getenv("WEBSERVER_PORT");
  sprintf(header, "%s:%s/identify", ip, port);
  
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
  int totalSize = raw.len;
 printf("Processing...\n");
  unsigned char copy[30000];
  int i = 0, j = 0, pos = 0;
  for(pos = 0; pos < totalSize; pos++) {
    fprintf(stderr, "pos %d\n", pos);
      if(raw.data[pos] == '\t'){
	copy[i] = '\0';
	data[j] = malloc(sizeof *data);
	data[j] = fp_print_data_from_data(copy, i+1);
	j++;
	i = 0;
	for(int x = 0; x < 30000; x++)
	  copy[x] = '\0';
      }else {
	copy[i] = raw.data[pos];
	i++;
      }
      
  }
  
    
  http_send_body(h, "OK", 2);

	fiobj_free(obj);

	startIdentification();
}

int main() {

  if (http_listen("3000", NULL,
                  .on_request = on_http_request) == -1) {
    perror(
        "ERROR: facil.io couldn't initialize HTTP service (already running?)\n");
    exit(1);
  }
  
  fio_start(.threads = 1, .workers = 1);
  fio_cli_end();
  return 0;
}


