# Share files
This is a decentralized file storage system


# TODO

INIT FILES
----
  - [x] Read all files
  - [ ] Create list of files and their destination 
  (ASK HOW THE HELL THE NETWORK SHOULD WORK)
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


SERVER CONNECTION
----
  - Hash amount --> 
  - <-- 0
  - For hash amount:
    - Hash --> 
    - <-- 0
    - Bytes for hash --> 
    - <-- 0


PROCESSES
----
  - [ ] Read all the files:
    - [ ] Compress, encrypt, split and hash
    - [ ] Make a list of those hashes in a file
  - [ ] Start TCP server and listen for requests
    - [ ] When a request for a file comes in:
      - [ ] Check all the hashes and if one coincides then send that file
      - [ ] If you don't have the file send a request for 
        that file to everyone you know in the DHT 
        with the first requester as the direction.
  - [ ] Start file listener
    - [ ] When a file changes or is added handle the file
    - [ ] Update your file list


DO NOW
----
  - [x] Full function to handle entire file: 
  (compress, encrypt, split, hash)
  - [ ] Fix hashing function
  - [ ] Write function to send a group of bytes
  - [ ] Some not read all the files at once.
