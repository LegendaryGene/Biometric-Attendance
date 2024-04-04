package db

import (
	"log"

	"gorm.io/driver/sqlite"
	"gorm.io/gorm"
    "github.com/ShivamIITK21/Biometric-Attendance/server/models"
)



func Connect() *gorm.DB{
    db, err := gorm.Open(sqlite.Open("test.db"), &gorm.Config{})
    if err != nil {
        log.Fatal("error in opening db")
    }

    db.AutoMigrate(&models.Admin{})
    db.AutoMigrate(&models.User{})
    db.AutoMigrate(&models.Logs{})

    admin := models.Admin{Password: "1234"}
    db.Create(&admin)
    
    return db
}

var DB = Connect()
