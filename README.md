# FAT32-Win-API

This program demonstrates some of the simplest functions of a "user-mode" driver. It runs in user mode and uses the Win32 API to do low-level access to an USB driver. 

When the program is run the user is asked for a drive name (letter) then the program reads the boot sector and displays the following information:
- Bytes per sector
- Secotrs per Cluster
- Number of FATs
- Total number of logical secotrs

After this, the user can insert a filename. The program then searches for the filename in the driver and displays the following information:
- Starting cluster number
- File size
- Last access date 
