package main

import (
	"bufio"
	"flag"
	"fmt"
	"os"
	"strings"

	"github.com/ashwanirathee/Distrifein/beb"
)

func main() {
	id := flag.String("id", "", "Node ID")
	port := flag.String("port", "", "Listening port")
	peersStr := flag.String("peers", "", "Comma-separated peer list")
	flag.Parse()

	if *id == "" || *port == "" || *peersStr == "" {
		panic("Usage: --id=ID --port=PORT --peers=host:port,...")
	}

	peers := strings.Split(*peersStr, ",")
	eventChan := make(chan beb.Event, 100)

	// Event dispatcher
	broadcaster := beb.NewBroadcaster(*id, peers, func(msg beb.BroadcastMessage) {
		for _, peer := range peers {
			go beb.SendMessage(peer, msg)
		}
	})

	// Start TCP server
	go beb.StartServer(*port, func(event beb.Event) {
		eventChan <- event
	})

	// Input loop
	go func() {
		reader := bufio.NewReader(os.Stdin)
		for {
			fmt.Print(">> ")
			text, _ := reader.ReadString('\n')
			text = strings.TrimSpace(text)
			if text != "" {
				eventChan <- beb.UserInputEvent{Text: text}
			}
		}
	}()

	// Main event loop
	for event := range eventChan {
		broadcaster.HandleEvent(event)
	}
}
