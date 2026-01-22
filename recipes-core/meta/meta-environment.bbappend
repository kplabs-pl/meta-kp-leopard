toolchain_shared_env_script:prepend() {
    echo 'set OECORE_TUNE_CCARGS=${TUNE_CCARGS}' >> $script
}