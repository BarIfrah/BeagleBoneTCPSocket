cmake_minimum_required(VERSION 3.23)
project(BoneServer C)

set(CMAKE_C_STANDARD 99)


add_executable(BoneServer src/main.c)
add_executable(Client src/Client.c)

add_executable(FFTProcess src/FFT.c)

add_executable(Convolution src/Convolution.c)
add_executable(ImageIdentification src/ImageIdentification.c)

add_subdirectory (include)
add_subdirectory (src)
