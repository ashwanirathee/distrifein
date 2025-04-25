package beb

import (
	"encoding/gob"
	"fmt"
	"net"
)

func StartServer(port string, emit func(Event)) {
	ln, err := net.Listen("tcp", ":"+port)
	if err != nil {
		panic(err)
	}
	for {
		conn, err := ln.Accept()
		if err != nil {
			fmt.Println("Connection error:", err)
			continue
		}
		go handleConnection(conn, emit)
	}
}

func handleConnection(conn net.Conn, emit func(Event)) {
	defer conn.Close()
	dec := gob.NewDecoder(conn)
	var msg BroadcastMessage
	err := dec.Decode(&msg)
	if err == nil {
		emit(MessageReceivedEvent{Msg: msg})
	}
}

func SendMessage(addr string, msg BroadcastMessage) {
	conn, err := net.Dial("tcp", addr)
	if err != nil {
		fmt.Printf("Dial error to %s: %v\n", addr, err)
		return
	}
	defer conn.Close()

	enc := gob.NewEncoder(conn)
	enc.Encode(msg)
}
