package models


type Admin struct{
    Password string
}

type User struct{
    RollNo  string
    PhoneNo string
    Id     uint8 
    CreatedOn   string
}

type Logs struct{
    RollNo  string
    EnterOn string
}
