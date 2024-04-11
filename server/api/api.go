package api

import (
	// "fmt"
	"fmt"
	"log"
	"net/http"
	"time"

	"github.com/ShivamIITK21/Biometric-Attendance/server/models"

	// "github.com/ShivamIITK21/Biometric-Attendance/server/db"
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
	ReadBufferSize:  1024,
	WriteBufferSize: 1024,
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

func handle(w http.ResponseWriter, r *http.Request) {
	conn, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		fmt.Println("Error upgrading connection")
		return
	}
	// log.Println("here")

	defer conn.Close()

	db, err := gorm.Open(sqlite.Open("test.db"), &gorm.Config{})
	if err != nil {
		log.Fatal("error in opening db")
	}

	for {
		time.Sleep(1*time.Second)

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

// func (api *API) StartAPI() {
// 	server := http.NewServeMux()
// 	server.HandleFunc("/ws", handle)
// 	log.Println("Starting API....")
// 	log.Fatal(http.ListenAndServe(":8080", nil))
// }

func (api *API) StartAPI() {
	http.HandleFunc("/ws", handle)
	log.Println("Starting API....")
	log.Fatal(http.ListenAndServe(":8080", nil))
}

// func handler(w http.ResponseWriter, r *http.Request) {
// 	fmt.Println("here")
// 	fmt.Fprintf(w, "Hello, %q", r.URL.Path)
// }
