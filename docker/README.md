## libfprint dockerized

Install docker and docker-compose on you Raspberry pi.

Create the folder `/tmp/docker/volumes/mysql`. It will be used as a volume by the MySQL container, where it will store the database files.

Download or clone this project, then:

```
root# cd ./raspi-libfprint/docker
root# docker-compose up -d
```
