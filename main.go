package main

import (
	"fmt"
	"net/http"
)

func main() {
	http.HandleFunc("/upload", handleUpload)
	http.HandleFunc("/download", handleDownload)

	fmt.Println("Starting server at http://localhost:8081")
	err := http.ListenAndServe(":8081", nil)
	if err != nil {
		fmt.Println("Server error:", err)
	}
}
