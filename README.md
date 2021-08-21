# Dezentralized file storage
This is a simple decentralized file storage system.
It works by sharing your files with other people so that your files are in a dezentralized cloud.

All files are encrypted and cut into blocks to ensure privacy.
If you want to understand how it works more in detail you can read [this article](https://ipfs.io/#how).

# Use cases
This can be used if you are looking to free up some space or you want to sync files between different computers. You can create your own network of devices your connect to the public one.

_This system hasn't been tested enough. Don't use it for anything really important
# How to use it
## Setup
1. Execute the setup script to generate a service file.
2. Make sure port `8080` is open.
3. Indicate what folder you want you sync.

## Using it
1. `.f` files will replace your original files. **Do not alter these files. They contain your original files hashes. If you loose those hashes your files is gone.**
2. To access your original files simply access (read or open the file) the `.f` file and the original file will be generated.


# TODOS

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
  - [X] Read all the files:
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
  - [ ] Somehow not read all the files at once.
