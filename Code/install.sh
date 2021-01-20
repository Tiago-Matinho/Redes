gcc -Wall broker.c library.h library.c -g -o broker
gcc -Wall sensor.c library.h library.c -g -o sensor
gcc -Wall public_cli.c library.h library.c -g -o public_cli
gcc -Wall admin_cli.c library.h library.c -g -o admin_cli
