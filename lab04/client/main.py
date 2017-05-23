from __future__ import print_function

import random
import time

import grpc

import hospital_pb2
import hospital_pb2_grpc

def add_examination(stub):
    examination = hospital_pb2.ExaminationResult()
    examination.Date = input("Podaj date badania: ")
    examination.Commisioner = input("Lekarz zlecajacy: ")
    idstr = input("ID pacjenta: ")
    if idstr != "" :
        examination.Person.Id = int(idstr)
    examination.Person.FirstName = input("Imie pacjenta: ")
    examination.Person.LastName = input("Nazwisko pacjenta: ")
    
    i = int(input("Ile parametrow chcesz podac?: "))
    
    while i > 0 :
        parameter = hospital_pb2.Parameter()
        parameter.Name = input("Podaj nazwe parametru: ")
        parameter.Value = float(input("Podaj wartosc parametru: "))
        parameter.Unit = input("Podaj jednostke parametru: ")
        examination.Parameters.extend([parameter])
        i = i - 1
    
    future_retval = stub.AddExamination.future(examination) #ASYNCHRONICZNE
    retval = future_retval.result()
    print(retval)
    
    
    

def get_examination(stub):
    request = hospital_pb2.ExaminationResult()
    request.Date = input("Podaj date badania: ")
    request.Commisioner = input("Lekarz zlecajacy: ")

    for result in stub.GetExamination(request):
        print(result)
        
def get_examinations_by_userid(stub):
    request = hospital_pb2.ExaminationResult()
    request.Person.Id = int(input("ID pacjenta: "))

    for result in stub.GetExamination(request):
        print(result)


def main(args):
    channel = grpc.insecure_channel('localhost:50051')
    stub = hospital_pb2_grpc.HospitalStub(channel)
    while True :
        i = int(input("1: Dodaj badanie\n2: Znajdz badanie(lekarz)\n3: Pokaz badania pacjenta(uzytkownik)\n"))
        if i == 1:
            add_examination(stub)
        elif i == 2:
            get_examination(stub)
        elif i == 3:
            get_examinations_by_userid(stub)
        else:
            print("Niewlasciwy wybor!")
	
    return 0

if __name__ == '__main__':
    import sys
    sys.exit(main(sys.argv))
