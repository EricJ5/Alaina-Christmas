package main

import (
	"embed"
	"io/fs"
	"log"
	"net/http"
	"strings"

	"github.com/gin-gonic/gin"
	"github.com/gorilla/websocket"
)

//go:embed frontend/dist/*
var embedFrontend embed.FS

var upgrader = websocket.Upgrader{
	ReadBufferSize:  1024,
	WriteBufferSize: 1024,
	CheckOrigin: func(r *http.Request) bool {
		return true
	},
}

func handleWebSocket(c *gin.Context) {
	conn, err := upgrader.Upgrade(c.Writer, c.Request, nil)
	if err != nil {
		log.Printf("WebSocket upgrade error: %v", err)
		return
	}
	SetUpNewClient(conn)

}

func main() {
	cApiStart()
	router := gin.Default()
	distFS, err := fs.Sub(embedFrontend, "frontend/dist")
	if err != nil {
		log.Fatalf("Failed to sub fs %v", err)
	}

	staticFS := http.FS(distFS)

	router.GET("/ws", handleWebSocket)

	api := router.Group("/api")
	{
		api.GET("/status", func(c *gin.Context) {
			c.JSON(http.StatusOK, gin.H{"status": "running"})
		})
	}
	router.NoRoute(func(c *gin.Context) {
		path := c.Request.URL.Path

		filePath := strings.TrimPrefix(path, "/")

		if file, err := distFS.Open(filePath); err == nil && filePath != "" {
			file.Close()
			c.FileFromFS(path, staticFS)
			return
		}

		if strings.HasPrefix(path, "/api") || path == "/ws" {
			c.JSON(http.StatusNotFound, gin.H{"error": "Not Found"})
			return
		}

		reactData, err := fs.ReadFile(distFS, "index.html")
		if err != nil {
			c.String(http.StatusNotFound, "Frontend build files missing.")
			return
		}
		c.Data(http.StatusOK, "text/html; charset=utf-8", reactData)
	})
	router.Run(":8080")

}
