all: server

server:
	g++ main.cpp ./gen/hospital.grpc.pb.cc ./gen/hospital.pb.cc -std=c++11 -o main -lgrpc++ -lprotobuf -lpthread -ldl -g
