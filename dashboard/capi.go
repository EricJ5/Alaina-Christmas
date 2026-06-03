package main

import (
	"encoding/binary"
	"encoding/json"
	"errors"
	"github.com/gorilla/websocket"
	"io"
	"log"
	"math"
	"net"
)

type Opcode int32

var globalBroker *Broker
var backendQueue *BackendQueue

const (
	GET_VOLTAGE Opcode = 1
	SET_VOLTAGE        = 2
	GET_VOLUME         = 3
	SET_VOLUME         = 4
)

const SockPath = "/tmp/dashboard_socket.sock"

type BackendPacket struct {
	Opcode  Opcode
	Size    uint32
	Payload []byte
}

func float64encode(f float64) []byte {
	buf := make([]byte, 4)
	bits := math.Float64bits(f)
	binary.NativeEndian.PutUint64(buf, bits)
	return buf

}

type OutboundPacket struct {
	EventName string `json:"eventName"`
	Data      any    `json:"data,omitempty"`
}
type InboundPacket struct {
	EventName string          `json:"eventName"`
	Data      json.RawMessage `json:"data,omitempty"`
}

func (pkt BackendPacket) packetToJSON() []byte {
	var eventName string
	var dataAny any
	switch pkt.Opcode {
	case SET_VOLTAGE:
		eventName = "setVoltage"
		if len(pkt.Payload) == 8 {
			bits := binary.NativeEndian.Uint64(pkt.Payload)
			dataAny = math.Float64frombits(bits)
			break
		}
		log.Printf("packet is wrong size for voltageData")
		return nil
	case GET_VOLTAGE:
		eventName = "getVoltage"
	case SET_VOLUME:
		eventName = "setVolume"
		if len(pkt.Payload) == 8 {
			bits := binary.NativeEndian.Uint64(pkt.Payload)
			dataAny = math.Float64frombits(bits)
			break
		}
		log.Printf("packet is wrong size for volumeData")
		return nil
	case GET_VOLUME:
		eventName = "getVolume"
	}

	request := OutboundPacket{
		EventName: eventName,
		Data:      dataAny,
	}
	jsonData, err := json.Marshal(request)
	if err != nil {
		log.Printf("Error converting cpacket to json: %v", err)
		return nil
	}
	return jsonData
}

type VoltageData struct {
	Voltage float64 `json:"voltage"`
}
type VolumeData struct {
	Volume float64 `json:"volume"`
}

func (pkt *BackendPacket) JSONToPacket(JSON []byte) {
	var inbound InboundPacket
	var code Opcode
	var size uint32
	var load []byte
	err := json.Unmarshal(JSON, &inbound)
	if err != nil {
		log.Printf("error unmarshaling json: %v", err)
	}
	switch inbound.EventName {
	case "setVoltage":
		code = SET_VOLTAGE
		size = 8
		var voltage VoltageData
		err := json.Unmarshal(inbound.Data, &voltage.Voltage)
		if err != nil {
			log.Printf("error converting Json to cpacket: %v", err)
			return
		}
		buf := make([]byte, 8)
		bits := math.Float64bits(voltage.Voltage)
		binary.NativeEndian.PutUint64(buf, bits)
		load = buf
	case "getVoltage":
		code = GET_VOLTAGE

	case "setVolume":
		code = SET_VOLUME
		size = 8
		var volume VolumeData
		err := json.Unmarshal(inbound.Data, &volume.Volume)
		if err != nil {
			log.Printf("error converting Json to cpacket: %v", err)
			return
		}
		buf := make([]byte, 8)
		bits := math.Float64bits(volume.Volume)
		binary.NativeEndian.PutUint64(buf, bits)
		load = buf
	case "getVolume":
		code = GET_VOLUME
	}
	pkt.Opcode = code
	pkt.Size = size
	pkt.Payload = load
}

type BackendQueue struct {
	conn net.Conn
	c    chan *BackendPacket
}
type FrontendQueue struct {
	conn *websocket.Conn
	c    chan []byte
}

func connect() (net.Conn, error) {
	conn, err := net.Dial("unix", SockPath)
	if err != nil {
		log.Printf("error connecting to socket: %v", err)
	}
	return conn, err
}
func BackendWriter(bq *BackendQueue) {
	c := bq.c
	conn := bq.conn
	for pkt := range c {
		err := func() error {
			if err := binary.Write(conn, binary.NativeEndian, pkt.Opcode); err != nil {
				return err
			}
			if err := binary.Write(conn, binary.NativeEndian, pkt.Size); err != nil {
				return err
			}
			n, err := conn.Write(pkt.Payload)
			if err != nil {
				return err
			}
			if n < int(pkt.Size) {
				return errors.New("under write")
			}
			return nil
		}()
		if err != nil {
			log.Printf("error writing packet: %v", err)
		}
	}
	return
}

func BackendReader(bq *BackendQueue) {
	conn := bq.conn
	for {
		var pkt BackendPacket
		binary.Read(conn, binary.NativeEndian, &pkt.Opcode)

		binary.Read(conn, binary.NativeEndian, &pkt.Size)

		pkt.Payload = make([]byte, pkt.Size)
		if _, err := io.ReadFull(conn, pkt.Payload); err != nil {
			log.Printf("Failed to read full packet payload: %v\n", err)
			return
		}
		globalBroker.broadcast <- pkt.packetToJSON()

	}
}

func SetUpNewClient(conn *websocket.Conn) {
	fq := &FrontendQueue{
		conn: conn,
		c:    make(chan []byte, 128),
	}

	globalBroker.register <- fq
	go FrontendWriter(fq)
	go FrotendReader(fq)

}
func FrontendWriter(q *FrontendQueue) {
	c := q.c
	conn := q.conn

	for json := range c {
		if json == nil {
			continue
		}
		log.Printf(string(json))
		err := conn.WriteMessage(websocket.TextMessage, json)
		if err != nil {
			log.Printf("Error writing json to websocket: %v")

		}
	}
}

func FrotendReader(fq *FrontendQueue) {
	c := backendQueue.c
	conn := fq.conn
	defer func() {
		globalBroker.unregister <- fq
		conn.Close()
	}()
	for {
		messageType, payload, err := conn.ReadMessage()
		if err != nil {
			log.Printf("client disconnected: %v", err)
			return
		}

		if messageType != websocket.TextMessage {
			continue

		}
		log.Printf(string(payload))
		var pkt BackendPacket
		pkt.JSONToPacket(payload)
		c <- &pkt
	}
}

func cApiStart() {

	conn, err := net.Dial("unix", SockPath)
	if err != nil {
		log.Printf("failed to connect to socket for C: %v", err)
		return
	}
	globalBroker = CreateBroker()
	backendQueue = &BackendQueue{
		conn: conn,
		c:    make(chan *BackendPacket, 128),
	}
	go globalBroker.start()
	go BackendReader(backendQueue)
	go BackendWriter(backendQueue)

}
