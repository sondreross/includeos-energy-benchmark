#!/usr/bin/env bash
set -e

# Create directory structure
mkdir -p iso/boot/grub

# Copy only the IncludeOS binary (no chainloader)
cp freq_profiling.elf.bin iso/boot/freq_profiling.elf.bin

# Create GRUB config for direct boot
cat > iso/boot/grub/grub.cfg << 'EOF'
set timeout=10
set default=0

menuentry "IncludeOS - Direct Boot (No Chainloader)" {
    multiboot /boot/freq_profiling.elf.bin
}

EOF

# Create bootable ISO
grub-mkrescue --modules="multiboot iso9660 configfile normal" -o freq_profiling.iso iso

echo "ISO created: freq_profiling.iso"
echo "Files in ISO structure:"
find iso/ -type f

# Clean up temporary ISO structure
rm -rf iso
