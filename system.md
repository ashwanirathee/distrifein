#### Current Mind Model:

```
+-------------------------+
|        Client UI        |
|  (Web App / Mobile App) |
+-----------+-------------+
            |
            ▼
+---------------------------+
|     API Gateway / LB      |  ← Auth, Rate Limit, Route
+-----------+---------------+
            |
+-----------▼---------------------------------------------------------------+
|                                Backend Services                           |
|----------------------------------------------------------------------------|
|  Auth Service     |  Posts Service   |  Comments Service  |  Feed Service  |
|-------------------|------------------|---------------------|----------------|
| User Reg/Login    | Create/Fetch     | Add/Fetch           | Personalized   |
| JWT Tokens        | Upvotes/Downvotes| Votes & Nesting     | or Global Feed |
+----------------------------------------------------------------------------+
            |
            ▼
+------------------------------+      +-----------------------------+
|     Distributed Database     |◄────►|    Caching Layer (Redis)    |
| (e.g., Cassandra / CouchDB)  |      | For hot feeds & votes cache |
+------------------------------+      +-----------------------------+
            |
            ▼
+----------------------------+
|  Distributed File Storage  | ← Optional: for media/images (e.g., IPFS, S3)
+----------------------------+
```


- `go run .` runs the main module in the folder
- `go help` for other modules
- `go get rsc.io/quote ` to get the quote module
- ` go mod tidy` to find and download dependencies that i mention


export PATH="$PATH:/Applications/Docker.app/Contents/Resources/bin/"

- `curl -X POST localhost:PORT/upload` sends a post request
- `curl "http://localhost:PORT/download?cid=cid_value"
` this will download the file
- `curl -X POST -F "file=@hello.txt" http://localhost:8081/upload` for uploading a file, just hello.txt to hello.png to upload a png file
- `--output -` to show the file in the terminal with curl request.

####  IPFS Swarm Connection
- To connect the swarm memory manually: use `docker exec ipfs_node2 ipfs id` to get the port of the node
- `docker exec ipfs_node1 ipfs swarm connect ip_path_otherthan_local_host_which_has_p2p_tcp_ip4` in path
- To make this connection that were manually done, we can make them persistent after restarts:
    - Use IPFS Peering (add entries to the peering.peers section in config).
    - script the swarm connect call in your Docker entrypoint or startup.

#### Go based best effort broadcast with tcp
- `go run ./examples/beb.go --id=node1 --port=8001 --peers=localhost:8002,localhost:8003` to run the first node and changing the details to run 3 or more nodes.
- The programming style for broadcaster.go is very similar I saw in Julia community 
- Switching to C++ as more comfortable with C++ and the performance is better