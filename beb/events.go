package beb

type Event interface{}

type UserInputEvent struct {
	Text string
}

type MessageReceivedEvent struct {
	Msg BroadcastMessage
}

type BroadcastMessage struct {
	ID   string
	From string
	Body string
}
