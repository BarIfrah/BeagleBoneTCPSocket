# BeagleBoneTCPSocket

##About
This project greates a TCP socket to communicate between server and client, when server is a beaglebone card.\
On the card, messages go through a identification process to decide what kinf od an image is shown.\
When done, message goes through a UART channel on the card and back to the client.

## Compile and run
Compile:\
Compile each file manually (using GCC compiler) or build entire progect using CMake.\
To compile manually, use:\
```
gcc -Wall <filename.c> -o <exeFileName>
```

## Project Structure
The folder structure of this app is explained below:

| Name | Description |
| ----------------------------- | --------------------------------------------------------------------------------------------- |
| **src**                       | Contains source code (.c) files.                                                              |
| **include**                   | Contains header (.h) files.                                                                   |
| **src/main**                  | Main code of program - creates socket, shared memory and contains project's logic.            |
| **FFT.c / .h**            | Calculates FFT for a given image string.                                                      |
| **Convolution.c / .h**        | Calculates convolution for a given image string.                                              |
| **ImageIdentification.c / .h**| Decides what kind of image is shown for a given image string.                                 |
| **Message.c /.h**             | Contains a struct (wrapper attributes) for a given image string.                              |
| **SharedMemClient.c /.h**     | Contains all of identical client's logic.                                                     |
