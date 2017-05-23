#include <cstdlib>
#include <iostream>
#include <list>

#include <grpc++/grpc++.h>
#include "./gen/hospital.grpc.pb.h"
#include "./gen/hospital.pb.h"

using namespace std;

using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerAsyncWriter;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerCompletionQueue;
using grpc::Status;

list<ExaminationResult> lista;

class HospitalImpl final {
	public:
  ~HospitalImpl() {
    server_->Shutdown();
    cq_->Shutdown();
  }

  void Run() {
    std::string server_address("0.0.0.0:50051");

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service_);
    cq_ = builder.AddCompletionQueue();

    server_ = builder.BuildAndStart();
    std::cout << "Server listening on " << server_address << std::endl;

    HandleRpcs();
  }

 private:
  class CallData {
   public:
    CallData(Hospital::AsyncService* service, ServerCompletionQueue* cq, bool mf)
        : service_(service), cq_(cq), AddResponder_(&ctx_), status_(CREATE), mysterious_flag(mf), writer(&ctx_) {
      Proceed();
    }

    void Proceed() {
      if (status_ == CREATE) {
        status_ = PROCESS;
        iter = lista.begin();
        if (mysterious_flag)
			service_->RequestAddExamination(&ctx_, &request_, &AddResponder_, cq_, cq_, this);
		else
			service_->RequestGetExamination(&ctx_, &request_, &writer, cq_, cq_, this);
        
      } else if (status_ == PROCESS) {
		if (mysterious_flag) {
			//AddExamination
			new CallData(service_, cq_, true); //Trick ;D
			
			lista.push_back(request_);
			
			//Tu mamy kod co wpisuje do reply_
			AddReply_.set_retval(true);

			status_ = FINISH;
			AddResponder_.Finish(AddReply_, Status::OK, this);
		} else {
			//GetExamination
			ExaminationResult *GetReply_;
			bool good = true;
			
			while (iter != lista.end()) {
				GetReply_ = &*iter;
				good = true;
				if(!request_.date().empty())
					if(request_.date() != GetReply_->date()) {
						good = false;
					}
						
				if(!request_.commisioner().empty())
					if(request_.commisioner() != GetReply_->commisioner()) {
						good = false;
					}
						
				if(request_.person().id() != 0)
					if(request_.person().id() != GetReply_->person().id()) {
						good = false;
					}
				if(!good)
					iter++;
				else
					break;
			}
			
			if (iter == lista.end()) {
				new CallData(service_, cq_, false); //Trick ;D
				status_ = FINISH;
				if (!mysterious_flag)
					writer.Finish(Status::OK, this);
				status_ = DESTROY;
				return;
			}
			
			//Tu mamy kod co wpisuje do reply_
			writer.Write(*GetReply_, this);

			//uwaga na to pod spodem, musze miec jeszcze raz tag by zrobic kolejnego write'a
			//czyli ten finish trzeba owinac w jakis conditional
			if(++iter == lista.end()) {
				new CallData(service_, cq_, false); //Trick ;D
				status_ = FINISH;
			}

		}
      } else if (status_ == FINISH){
        if (!mysterious_flag)
			writer.Finish(Status::OK, this);
		status_ = DESTROY;
      } else {
		delete this;
	  }
    }

   private:
    bool mysterious_flag;
    Hospital::AsyncService* service_;
    ServerCompletionQueue* cq_;
    ServerContext ctx_;

    ExaminationResult request_;
    
    AddExaminationReturnMsg AddReply_;
    ServerAsyncResponseWriter<AddExaminationReturnMsg> AddResponder_;
    ServerAsyncWriter<ExaminationResult> writer;
    
    list<ExaminationResult>::iterator iter;

    enum CallStatus { CREATE, PROCESS, FINISH, DESTROY };
    CallStatus status_;
  };

  void HandleRpcs() {
    new CallData(&service_, cq_.get(), true);
    new CallData(&service_, cq_.get(), false);
    void* tag;  // uniquely identifies a request.
    bool ok;
    while (true) {
      cq_->Next(&tag, &ok);
      static_cast<CallData*>(tag)->Proceed();
    }
  }

  std::shared_ptr<ServerCompletionQueue> cq_;
  Hospital::AsyncService service_;
  std::unique_ptr<Server> server_;
	
};


int main() {
	Person *me = new Person();
			me->set_id(1);
			me->set_firstname("Patryk");
			me->set_lastname("Duda");
			
			
	ExaminationResult *GetReply_ = new ExaminationResult();
	GetReply_->set_date("04-05-2017");
	GetReply_->set_commisioner("Eugieniusz Klopotek");
	GetReply_->set_allocated_person(me);
	Parameter *p1 = GetReply_->add_parameters();
	Parameter *p2 = GetReply_->add_parameters();
	
	p1->set_name("sod");
	p1->set_value(11.1);
	p1->set_unit("mg");
	
	p2->set_name("miedz");
	p2->set_value(0.11);
	p2->set_unit("mg");
	
	lista.push_front(*GetReply_);
	delete GetReply_;
	
	HospitalImpl server;
	server.Run();
	
	return 0;
	
	
}
