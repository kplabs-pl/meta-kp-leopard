# If debug output is needed change 0 -> 1
# This option also enables FSBL debug output.
# It adds PMU compiler flags "-DXPFW_DEBUG_DETAILED -DDEBUG_MODE" which can be added alternatively below.
XSCTH_BUILD_DEBUG = "0"

# If more precise debug messages are needed add compiler flag:
# "-DPM_LOG_LEVEL=X" - enables print based debug functions, X in range 1-4. For higher levels, PMU occupies more memory.
YAML_COMPILER_FLAGS:append = ""