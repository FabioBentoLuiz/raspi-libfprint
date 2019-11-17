# raspi-libfprint
A fingerprint recognition demo implementation for Raspberry pi using libfprint, Java Spring Boot and MySQL.
* database folder:

Contains the SQL script with the expected database structure and user to run the demo.

If you run the dockerized version, the MySQL container loads this script automatically on startup. 

* docker folder:

Contains the Dockerfiles used to create the docker images for [enrollment](https://hub.docker.com/r/fabiobentoluiz/libfprint_enroll), [verification](https://hub.docker.com/r/fabiobentoluiz/libfprint_verify), [identification](https://hub.docker.com/r/fabiobentoluiz/libfprint_identify) and [Spring Boot rest/web](https://hub.docker.com/r/fabiobentoluiz/libfprint_web).

It contains also the `docker-compose.yml` file to start the containers all together, including MySQL.

* libfprint folder

Contains the .c source files used for enrollment, verification and identification. They are based on the original examples provided by the libfprint project [here](https://github.com/freedesktop/libfprint/tree/master/examples).

* spring-libfprint folder

Contains the Spring Boot application with the rest service (that communicate with the other libfprint services) and web components (with the pages to register users and simulate the enrollment, verification and identification).
#
#
