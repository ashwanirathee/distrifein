# Distrifein

Distributed, Decentralized, Fault-Tolerant Media Platform in C++

#### Current Features:
- Peer-to-Peer Communication via TCP
- Best Effort Broadcast over Peer-to-Peer layer
- Failure Detector using heartbeat-based method
- Support for broadcasting text, images(ppm currently supported)
- Event-driven intra-layer communication with modular layered architecture with recursive mutexes
- Reliable Broadcast
- Uniform Reliable Broadcast

#### Example: image transfer via uniform reliable broadcast
Displays the image of the uniform reliable broadcast protocol in action where I specify the file to be sent. It first puts the image in pending for itselfs and then waits for acks from the other nodes to deliver it. SID is the current sender id and Org SID is the original sender id. We wait for acks for all the correct processes to deliver the image before being saved to the disk:

![](./assets/urb2.png)

#### Results:
- I had 3 processes running on my local machine, each with a different port number. The processes were able to communicate with each other and transfer images using the uniform reliable broadcast protocol. Each node had a unique ID and a folder to store the images received. The images were transferred successfully which is shown below:

![](./assets/urb_image_transfer.png)
![](./assets/node_0.png)

Some Logs are present here: https://gist.github.com/ashwanirathee/c7dcb605a0bfa88e53890b2812e95169 

#### References
This project is greatly influenced by course content from the graduate Distributed Systems class (CSE 232) at UCSC, taught by Prof. Mohsen Lesani.

- Notes: https://mohsenlesani.github.io/slugcse232/ 
