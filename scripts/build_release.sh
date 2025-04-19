(
    cd Vendor/modulee-engine-clib &&
        cargo build --release
) && (
    cd Builds/LinuxMakefile &&
        make CONFIG=Release -j$(nproc)
)
