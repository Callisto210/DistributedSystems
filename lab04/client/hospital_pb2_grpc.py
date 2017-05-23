# Generated by the gRPC Python protocol compiler plugin. DO NOT EDIT!
import grpc

import hospital_pb2 as hospital__pb2


class HospitalStub(object):

  def __init__(self, channel):
    """Constructor.

    Args:
      channel: A grpc.Channel.
    """
    self.AddExamination = channel.unary_unary(
        '/Hospital/AddExamination',
        request_serializer=hospital__pb2.ExaminationResult.SerializeToString,
        response_deserializer=hospital__pb2.AddExaminationReturnMsg.FromString,
        )
    self.GetExamination = channel.unary_stream(
        '/Hospital/GetExamination',
        request_serializer=hospital__pb2.ExaminationResult.SerializeToString,
        response_deserializer=hospital__pb2.ExaminationResult.FromString,
        )


class HospitalServicer(object):

  def AddExamination(self, request, context):
    context.set_code(grpc.StatusCode.UNIMPLEMENTED)
    context.set_details('Method not implemented!')
    raise NotImplementedError('Method not implemented!')

  def GetExamination(self, request, context):
    context.set_code(grpc.StatusCode.UNIMPLEMENTED)
    context.set_details('Method not implemented!')
    raise NotImplementedError('Method not implemented!')


def add_HospitalServicer_to_server(servicer, server):
  rpc_method_handlers = {
      'AddExamination': grpc.unary_unary_rpc_method_handler(
          servicer.AddExamination,
          request_deserializer=hospital__pb2.ExaminationResult.FromString,
          response_serializer=hospital__pb2.AddExaminationReturnMsg.SerializeToString,
      ),
      'GetExamination': grpc.unary_stream_rpc_method_handler(
          servicer.GetExamination,
          request_deserializer=hospital__pb2.ExaminationResult.FromString,
          response_serializer=hospital__pb2.ExaminationResult.SerializeToString,
      ),
  }
  generic_handler = grpc.method_handlers_generic_handler(
      'Hospital', rpc_method_handlers)
  server.add_generic_rpc_handlers((generic_handler,))
