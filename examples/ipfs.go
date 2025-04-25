package main

import (
	"fmt"
	"net/http"

	"github.com/ashwanirathee/Distrifein/ipfs"
)

func main() {
	http.HandleFunc("/upload", ipfs.handleUpload)
	http.HandleFunc("/download", ipfs.handleDownload)

	fmt.Println("Starting server at http://localhost:8081")
	err := http.ListenAndServe(":8081", nil)
	if err != nil {
		fmt.Println("Server error:", err)
	}
}
