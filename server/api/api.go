package api

import (
	"fmt"
	"log"
	"net/http"

	"github.com/gorilla/websocket"
)

type Sender struct {
	Conn *websocket.Conn
}

var upgrader = websocket.Upgrader{
	CheckOrigin: func(r *http.Request) bool {
		return true // Accepting all requests
	},
}

type Message struct {
	Msg string `json:"msg"`
}


func handle(w http.ResponseWriter, r *http.Request) {
	conn, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		fmt.Println("Error upgrading connection")
		return
	}
	defer conn.Close()

	for {
		_, msg, err := conn.ReadMessage()
		if err != nil {
			fmt.Println("Error reading message from connection")
		}
		go msgHandler(msg)
		res := Message{Msg: string(msg)}
		// fmt.Println(res)
		conn.WriteJSON(res)
	}
}

func msgHandler(msg []byte) {
	fmt.Println(string(msg))
}

func StartAPI() {
	http.HandleFunc("/ws", handle)
	log.Fatal(http.ListenAndServe(":8080", nil))
}
