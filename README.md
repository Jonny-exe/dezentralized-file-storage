# Share files
This is a decentralized file storage system


# TODO

INIT FILES
----
  - [x] Read all files
  - [ ] Create some initial users list to spread the message
  - [x] Create a hash for each file dependent on its content

COMPUTERS
----
  - [ ] Personal encryption key
  - [x] Identify with hashes
  - [ ] Have to be part of a group indentified by a hash
  - [ ] Have to update their IP address to DHT each day
  - [x] Have to run TCP server

NETWORK
----
  - [x] Has to be decentralized
  - [x] It has to have a DHT 
  (dezentralized hash table) to keep all the users and it's files
  - [x] TCP server

SERVER
----
  - [x] Listen on some port

PROCESSES
----
  - [ ] Read all the files:
    - [x] Compress, encrypt, split and hash
    - [x] Make a list of those hashes in a file
  - [x] Start TCP server and listen for requests
    - [x] When a request for a file comes in:
      - [x] Check all the hashes and if one coincides then send that file
      - [x] If you don't have the file send a request for 
        that file to everyone you know in the DHT 
        with the first requester as the direction.
  - [x] Start file listener
    - [x] When a file changes or is added handle the file
    - [x] Update your file list


DO NOW
----
  - [x] Full function to handle entire file: 
  (compress, encrypt, split, hash)
  - [x] Fix hashing function
  - [x] Write function to send a group of bytes
  - [ ] Some not read all the files at once.
