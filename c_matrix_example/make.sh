# build erpc_setup_tcp.cpp 

g++ -I../erpc_c/infra -I../erpc_c/port -I../erpc_c/transports -I../erpc_c/setup  -I../erpc_c/config -c ../erpc_c/setup/erpc_setup_tcp.cpp -o ../Debug/Darwin/erpc/obj/erpc_c/setup/erpc_setup_tcp.o -ggdb

# build erpc_setup_mbf.cpp

g++ -I../erpc_c/infra -I../erpc_c/port -I../erpc_c/transports -I../erpc_c/setup  -I../erpc_c/config -c ../erpc_c/setup/erpc_setup_mbf_static.cpp -o ../Debug/Darwin/erpc/obj/erpc_c/setup/erpc_setup_mbf_static.o -ggdb

g++ -I../erpc_c/infra -I../erpc_c/port -I../erpc_c/transports -I../erpc_c/setup  -I../erpc_c/config -c ../erpc_c/setup/erpc_setup_mbf_dynamic.cpp -o ../Debug/Darwin/erpc/obj/erpc_c/setup/erpc_setup_mbf_dynamic.o -ggdb 

rm ~/erpc/Debug/Darwin/erpc/obj/erpc_c/setup/erpc_arbitrated_client*

g++ -I../erpc_c/infra -I../erpc_c/port -I../erpc_c/transports -I../erpc_c/setup  -I../erpc_c/config -c service/erpc_matrix_multiply/erpc_matrix_multiply_server.cpp -o server.o -ggdb

gcc -I../erpc_c/infra -I../erpc_c/port -I../erpc_c/transports -I../erpc_c/setup  -I../erpc_c/config -c server_main.c  -o server_main.o -ggdb

g++ ../Debug/Darwin/erpc/obj/erpc_c/infra/*.o  ../Debug/Darwin/erpc/obj/erpc_c/setup/*.o ../Debug/Darwin/erpc/obj/erpc_c/port/*.o ../Debug/Darwin/erpc/obj/erpc_c/transports/*.o server.o server_main.o -o server -ggdb

g++ -I../erpc_c/infra -I../erpc_c/port -I../erpc_c/transports -I../erpc_c/setup  -I../erpc_c/config -c service/erpc_matrix_multiply/erpc_matrix_multiply_client.cpp -o client.o -ggdb

gcc -I../erpc_c/infra -I../erpc_c/port -I../erpc_c/transports -I../erpc_c/setup  -I../erpc_c/config -c client_main.c  -o client_main.o -ggdb

g++ ../Debug/Darwin/erpc/obj/erpc_c/infra/*.o  ../Debug/Darwin/erpc/obj/erpc_c/setup/*.o ../Debug/Darwin/erpc/obj/erpc_c/port/*.o ../Debug/Darwin/erpc/obj/erpc_c/transports/*.o client.o client_main.o -o client -ggdb


