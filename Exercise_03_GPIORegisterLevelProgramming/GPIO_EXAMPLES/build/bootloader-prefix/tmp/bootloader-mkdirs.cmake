# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/biu/esp/esp-idf/components/bootloader/subproject"
  "/home/biu/courses/MCU-v2_K26.1_Assignment/Exercise_03_GPIORegisterLevelProgramming/GPIO_EXAMPLES/build/bootloader"
  "/home/biu/courses/MCU-v2_K26.1_Assignment/Exercise_03_GPIORegisterLevelProgramming/GPIO_EXAMPLES/build/bootloader-prefix"
  "/home/biu/courses/MCU-v2_K26.1_Assignment/Exercise_03_GPIORegisterLevelProgramming/GPIO_EXAMPLES/build/bootloader-prefix/tmp"
  "/home/biu/courses/MCU-v2_K26.1_Assignment/Exercise_03_GPIORegisterLevelProgramming/GPIO_EXAMPLES/build/bootloader-prefix/src/bootloader-stamp"
  "/home/biu/courses/MCU-v2_K26.1_Assignment/Exercise_03_GPIORegisterLevelProgramming/GPIO_EXAMPLES/build/bootloader-prefix/src"
  "/home/biu/courses/MCU-v2_K26.1_Assignment/Exercise_03_GPIORegisterLevelProgramming/GPIO_EXAMPLES/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/biu/courses/MCU-v2_K26.1_Assignment/Exercise_03_GPIORegisterLevelProgramming/GPIO_EXAMPLES/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/biu/courses/MCU-v2_K26.1_Assignment/Exercise_03_GPIORegisterLevelProgramming/GPIO_EXAMPLES/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
