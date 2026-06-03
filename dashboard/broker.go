package main

import "log"

type Broker struct {
	broadcast  chan []byte
	register   chan *FrontendQueue
	unregister chan *FrontendQueue
	clients    map[*FrontendQueue]bool
}

func CreateBroker() *Broker {
	return &Broker{
		broadcast:  make(chan []byte, 128),
		register:   make(chan *FrontendQueue),
		unregister: make(chan *FrontendQueue),
		clients:    make(map[*FrontendQueue]bool),
	}
}
func (b *Broker) start() {
	for {
		select {
		case fq := <-b.register:
			b.clients[fq] = true
		case fq := <-b.unregister:
			exist := b.clients[fq]
			if exist {
				delete(b.clients, fq)
				close(fq.c)
			}
		case packet := <-b.broadcast:
			for fq := range b.clients {
				select {
				case fq.c <- packet:
				default:
					log.Println("user channel full")

				}
			}
		}
	}
}
