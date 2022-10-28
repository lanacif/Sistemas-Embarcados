# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/lenin/esp/esp-idf/components/bootloader/subproject"
  "C:/Users/lenin/OneDrive/Documentos/git/Sistemas-Embarcados/p5/single_read/build/bootloader"
  "C:/Users/lenin/OneDrive/Documentos/git/Sistemas-Embarcados/p5/single_read/build/bootloader-prefix"
  "C:/Users/lenin/OneDrive/Documentos/git/Sistemas-Embarcados/p5/single_read/build/bootloader-prefix/tmp"
  "C:/Users/lenin/OneDrive/Documentos/git/Sistemas-Embarcados/p5/single_read/build/bootloader-prefix/src/bootloader-stamp"
  "C:/Users/lenin/OneDrive/Documentos/git/Sistemas-Embarcados/p5/single_read/build/bootloader-prefix/src"
  "C:/Users/lenin/OneDrive/Documentos/git/Sistemas-Embarcados/p5/single_read/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/lenin/OneDrive/Documentos/git/Sistemas-Embarcados/p5/single_read/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
