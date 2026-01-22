# If debug output is needed change 0 -> 1.
# This option also enables PMU debug output.
XSCTH_BUILD_DEBUG = "0"

# If debug output is needed add compiler flags:
# "-DFSBL_DEBUG_INFO" - enables debug output.
# "-DFSBL_NAND_EXCLUDE_VAL" - (for example) excludes NAND to reduce FSBL size to fit in OCM,
#   because enabling debug commands increses FSBL size.
YAML_COMPILER_FLAGS:append = "-DFSBL_SECURE_EXCLUDE -DFSBL_NAND_EXCLUDE_VAL"
