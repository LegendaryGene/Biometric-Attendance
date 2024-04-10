package api

import (
	"fmt"
	"log"
	"net/http"

	"github.com/gorilla/websocket"
)

type API struct {
	data chan []byte
}

var upgrader = websocket.Upgrader{
	CheckOrigin: func(r *http.Request) bool {
		return true
	},
}

type Message struct {
	Msg string `json:"msg"`
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

func (api *API) StartAPI() {	
	server := http.NewServeMux()
	server.HandleFunc("/ws", api.handle)
	log.Fatal(http.ListenAndServe(":8080", nil))
}
