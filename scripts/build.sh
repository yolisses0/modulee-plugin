(
    cd Vendor/modulee-engine-clib &&
        cargo build
) && (
    cd Builds/LinuxMakefile &&
        make
)
