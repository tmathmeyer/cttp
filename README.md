see test/src/C/main.c for an example API using the cttp webserver

to build and run the example, do a full checkout of the repo (with submodules!),
then run

cd test/

make binary

./build/binary/http 8080

you should now be able to visit the urls:



http://localhost:8080/get/uuid/... => "HELLO WORLD: ..."

http://localhost:8080/test/file    => the test.txt file

http://localhost:8080/...          => a 404 page

where the ... are any string (excluding '/', '&', etc)

