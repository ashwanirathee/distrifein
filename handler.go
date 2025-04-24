package main

import (
	"bytes"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"net/http"

	shell "github.com/ipfs/go-ipfs-api"
)

var sh1 = shell.NewShell("ipfs1:5001") // IPFS Node 1
var sh2 = shell.NewShell("ipfs2:5001") // IPFS Node 2

func handleUpload(w http.ResponseWriter, r *http.Request) {
	fmt.Println("Upload handler called", r.Method)
	if r.Method == http.MethodPost {
		fmt.Fprintf(w, "Received a Post request")
		// fileContent := []byte("Hello from Go to IPFS")
		fileContent, err := ioutil.ReadAll(r.Body)
		if err != nil {
			http.Error(w, "could not read body", 400)
			return
		}

		cid, err := sh1.Add(bytes.NewReader(fileContent))
		if err != nil {
			log.Fatal(err)
		}
		fmt.Fprintf(w, "File added to IPFS with CID: %s", cid)
	} else {
		http.Error(w, "Invalid request method", http.StatusMethodNotAllowed)
	}
}

func handleDownload(w http.ResponseWriter, r *http.Request) {
	fmt.Println("Download handler called", r.Method)
	cid := r.URL.Query().Get("cid")

	if cid == "" {
		http.Error(w, "CID is required", http.StatusBadRequest)
		return
	}

	if r.Method == http.MethodGet {
		fmt.Fprintf(w, "Received a Get request")
		read, err := sh2.Cat(cid) // Replace with actual CID
		if err != nil {
			log.Fatal(err)
		}
		data, err := io.ReadAll(read)
		if err != nil {
			log.Fatal(err)
		}
		w.Write(data)
		// fmt.Fprintf(w, "File content: %s", string(data))
	} else {
		http.Error(w, "Invalid request method", http.StatusMethodNotAllowed)
	}
}
