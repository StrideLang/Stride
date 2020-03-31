cmake_minimum_required(VERSION 3.10)

set(PLATFORM_PATH "${CURRENT_CMAKE_SOURCE_DIR}/..")
set(ARDUINO_DIR arduino-1.6.8)
set(ARDUINO_BIN "${CURRENT_CMAKE_SOURCE_DIR}/${ARDUINO_DIR}/arduino")
set(ARDUINO_PORT "/dev/ttyACM0")

message("Using Arduino CLI: ${ARDUINO_BIN}")

#execute_process(COMMAND ${ARDUINO_BIN} --upload --board arduino:avr:leonardo --port ${ARDUINO_PORT} --pref sketchbook.path=${CURRENT_CMAKE_SOURCE_DIR}/sketchbook  template.ino)

message("done")
