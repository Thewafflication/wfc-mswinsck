# Runs one corpus program through wfc and compares what it prints with the
# expected text (test case TC-0005, the same check WFC runs for its corpus).
# Without a usable WFC the script prints SKIPPED, which CTest reports as a
# skipped test.
# Usage: cmake -DWFC=<wfc> -DINPUT=<.bas|.vbp> -DEXPECTED=<file>
#   [-DDEBUG_EXPECTED=<file>] -P run_corpus_case.cmake
# EXPECTED holds the standard output. DEBUG_EXPECTED holds the Immediate-window
# output (Debug.Print) and load warnings that wfc writes to stderr.
if(NOT WFC OR NOT EXISTS "${WFC}")
  message(STATUS "SKIPPED: set WFC_EXECUTABLE to wfc.exe to run this test")
  return()
endif()
execute_process(
  COMMAND "${WFC}" --interpret "${INPUT}"
  OUTPUT_VARIABLE actual
  ERROR_VARIABLE errors
  RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "wfc exited with ${result}: ${errors}")
endif()

# Compares two texts ignoring the line ending and the space around them.
function(check_text what expected actual)
  string(REPLACE "\r\n" "\n" expected "${expected}")
  string(REPLACE "\r\n" "\n" actual "${actual}")
  string(STRIP "${expected}" expected)
  string(STRIP "${actual}" actual)
  if(NOT actual STREQUAL expected)
    message(
      FATAL_ERROR
      "${what} mismatch\n--- expected ---\n${expected}\n"
      "--- actual ---\n${actual}"
    )
  endif()
endfunction()

if(DEFINED DEBUG_EXPECTED)
  file(READ "${DEBUG_EXPECTED}" debug_expected)
  check_text("debug output" "${debug_expected}" "${errors}")
endif()
file(READ "${EXPECTED}" expected)
check_text("corpus output" "${expected}" "${actual}")
