package api

import (
	"fmt"
	"github.com/ShivamIITK21/Biometric-Attendance/server/models"
	"log"
	"net/http"
	// "github.com/ShivamIITK21/Biometric-Attendance/server/db
	"github.com/gorilla/websocket"
	"gorm.io/driver/sqlite"
	"gorm.io/gorm"
)

type API struct {
	data chan []byte
}

var upgrader = websocket.Upgrader{
	CheckOrigin: func(r *http.Request) bool {
		return true
	},
}

// type Users struct {
// 	Users []models.User`json:"users"`
// }
// type Logs struct {
// 	Logs []models.Logs`json:"logs"`
// }

type Message struct {
	// Msg   string        `json:"msg"`
	Users []models.User `json:"users"`
	Logs  []models.Logs `json:"logs"`
}

func New(data chan []byte) *API {

	a := API{
		data: data,
	}
	return &a
}

func (api *API) handle(w http.ResponseWriter, r *http.Request) {
	conn, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		fmt.Println("Error upgrading connection")
		return
	}
	defer conn.Close()

	db, err := gorm.Open(sqlite.Open("test.db"), &gorm.Config{})
	if err != nil {
		log.Fatal("error in opening db")
	}

	for {

		// _, msg, err := conn.ReadMessage()
		// if err != nil {
		// 	fmt.Println("Error reading message from connection")
		// }
		// go msgHandler(msg)
		// res := Message{Msg: string(msg)}
		// fmt.Println(res)
		// conn.WriteJSON(res)

		var users []models.User
		var logs []models.Logs

		db.Find(&users)
		db.Find(&logs)

		res := Message{Users: users, Logs: logs}
		// go msgHandler()
		conn.WriteJSON(res)



		// select {
		// 	case data := <-api.data:
		// 		err := conn.WriteMessage(websocket.TextMessage, data)
		// 		if err != nil {
		// 			log.Println(err)
		// 			return
		// 		}
		// }
	}
}

// func msgHandler(msg ) {

// }

func (api *API) StartAPI() {
	server := http.NewServeMux()
	server.HandleFunc("/ws", api.handle)
	log.Fatal(http.ListenAndServe(":8080", nil))
}
