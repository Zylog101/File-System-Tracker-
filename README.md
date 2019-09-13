# File-System-Tracker-
Designed and built a system which walks through Inode, zone and directory map of MINIX operating system

Functionality:
* A DirectoryWalker that will traverse the directory tree of a given directory and produce all the subdirectories and files reachable from that directory as well as the inodes and zones allocated to those subdirectories and files.

* An iNodeBitMapWalker that reads the inode bit map and returns all the inodes that have been allocated.

* A ZoneBitMapWalker that reads the zone bit map and returns all the zones that have been allocated.
