// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: hospital.proto

#include "hospital.pb.h"
#include "hospital.grpc.pb.h"

#include <grpc++/impl/codegen/async_stream.h>
#include <grpc++/impl/codegen/async_unary_call.h>
#include <grpc++/impl/codegen/channel_interface.h>
#include <grpc++/impl/codegen/client_unary_call.h>
#include <grpc++/impl/codegen/method_handler_impl.h>
#include <grpc++/impl/codegen/rpc_service_method.h>
#include <grpc++/impl/codegen/service_type.h>
#include <grpc++/impl/codegen/sync_stream.h>

static const char* Hospital_method_names[] = {
  "/Hospital/AddExamination",
  "/Hospital/GetExamination",
};

std::unique_ptr< Hospital::Stub> Hospital::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  std::unique_ptr< Hospital::Stub> stub(new Hospital::Stub(channel));
  return stub;
}

Hospital::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel)
  : channel_(channel), rpcmethod_AddExamination_(Hospital_method_names[0], ::grpc::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_GetExamination_(Hospital_method_names[1], ::grpc::RpcMethod::SERVER_STREAMING, channel)
  {}

::grpc::Status Hospital::Stub::AddExamination(::grpc::ClientContext* context, const ::ExaminationResult& request, ::AddExaminationReturnMsg* response) {
  return ::grpc::BlockingUnaryCall(channel_.get(), rpcmethod_AddExamination_, context, request, response);
}

::grpc::ClientAsyncResponseReader< ::AddExaminationReturnMsg>* Hospital::Stub::AsyncAddExaminationRaw(::grpc::ClientContext* context, const ::ExaminationResult& request, ::grpc::CompletionQueue* cq) {
  return new ::grpc::ClientAsyncResponseReader< ::AddExaminationReturnMsg>(channel_.get(), cq, rpcmethod_AddExamination_, context, request);
}

::grpc::ClientReader< ::ExaminationResult>* Hospital::Stub::GetExaminationRaw(::grpc::ClientContext* context, const ::ExaminationResult& request) {
  return new ::grpc::ClientReader< ::ExaminationResult>(channel_.get(), rpcmethod_GetExamination_, context, request);
}

::grpc::ClientAsyncReader< ::ExaminationResult>* Hospital::Stub::AsyncGetExaminationRaw(::grpc::ClientContext* context, const ::ExaminationResult& request, ::grpc::CompletionQueue* cq, void* tag) {
  return new ::grpc::ClientAsyncReader< ::ExaminationResult>(channel_.get(), cq, rpcmethod_GetExamination_, context, request, tag);
}

Hospital::Service::Service() {
  AddMethod(new ::grpc::RpcServiceMethod(
      Hospital_method_names[0],
      ::grpc::RpcMethod::NORMAL_RPC,
      new ::grpc::RpcMethodHandler< Hospital::Service, ::ExaminationResult, ::AddExaminationReturnMsg>(
          std::mem_fn(&Hospital::Service::AddExamination), this)));
  AddMethod(new ::grpc::RpcServiceMethod(
      Hospital_method_names[1],
      ::grpc::RpcMethod::SERVER_STREAMING,
      new ::grpc::ServerStreamingHandler< Hospital::Service, ::ExaminationResult, ::ExaminationResult>(
          std::mem_fn(&Hospital::Service::GetExamination), this)));
}

Hospital::Service::~Service() {
}

::grpc::Status Hospital::Service::AddExamination(::grpc::ServerContext* context, const ::ExaminationResult* request, ::AddExaminationReturnMsg* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status Hospital::Service::GetExamination(::grpc::ServerContext* context, const ::ExaminationResult* request, ::grpc::ServerWriter< ::ExaminationResult>* writer) {
  (void) context;
  (void) request;
  (void) writer;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


