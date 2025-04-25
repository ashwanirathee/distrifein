package beb

import (
	"fmt"
	"sync"
)

type Broadcaster struct {
	ID        string
	Peers     []string
	Seq       int
	Delivered map[string]bool
	Mu        sync.Mutex
	Send      func(msg BroadcastMessage) // injected network sender
}

func NewBroadcaster(id string, peers []string, sendFunc func(BroadcastMessage)) *Broadcaster {
	return &Broadcaster{
		ID:        id,
		Peers:     peers,
		Send:      sendFunc,
		Delivered: make(map[string]bool),
	}
}

func (b *Broadcaster) HandleEvent(event Event) {
	switch e := event.(type) {
	case UserInputEvent:
		msg := BroadcastMessage{
			ID:   fmt.Sprintf("%s:%d", b.ID, b.Seq),
			From: b.ID,
			Body: e.Text,
		}
		b.Seq++
		b.Broadcast(msg)
		b.Deliver(msg)

	case MessageReceivedEvent:
		b.Deliver(e.Msg)
	}
}

func (b *Broadcaster) Broadcast(msg BroadcastMessage) {
	for _, peer := range b.Peers {
		go b.SendTo(peer, msg)
	}
}

func (b *Broadcaster) SendTo(peer string, msg BroadcastMessage) {
	b.Send(msg) // wrapped network sender
}

func (b *Broadcaster) Deliver(msg BroadcastMessage) {
	b.Mu.Lock()
	defer b.Mu.Unlock()

	if b.Delivered[msg.ID] {
		return
	}
	b.Delivered[msg.ID] = true
	fmt.Printf("📩 Delivered [%s] from %s: %s\n", msg.ID, msg.From, msg.Body)
}
