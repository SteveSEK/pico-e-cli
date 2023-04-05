# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/ssekim/work-pico/pico-e/pico-e-cli/libraries/pico-sdk/tools/pioasm"
  "/home/ssekim/work-pico/pico-e/pico-e-cli/build/pioasm"
  "/home/ssekim/work-pico/pico-e/pico-e-cli/build/pico-sdk/src/rp2_common/cyw43_driver/pioasm"
  "/home/ssekim/work-pico/pico-e/pico-e-cli/build/pico-sdk/src/rp2_common/cyw43_driver/pioasm/tmp"
  "/home/ssekim/work-pico/pico-e/pico-e-cli/build/pico-sdk/src/rp2_common/cyw43_driver/pioasm/src/PioasmBuild-stamp"
  "/home/ssekim/work-pico/pico-e/pico-e-cli/build/pico-sdk/src/rp2_common/cyw43_driver/pioasm/src"
  "/home/ssekim/work-pico/pico-e/pico-e-cli/build/pico-sdk/src/rp2_common/cyw43_driver/pioasm/src/PioasmBuild-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/ssekim/work-pico/pico-e/pico-e-cli/build/pico-sdk/src/rp2_common/cyw43_driver/pioasm/src/PioasmBuild-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/ssekim/work-pico/pico-e/pico-e-cli/build/pico-sdk/src/rp2_common/cyw43_driver/pioasm/src/PioasmBuild-stamp${cfgdir}") # cfgdir has leading slash
endif()
