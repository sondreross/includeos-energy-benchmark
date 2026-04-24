#!/usr/bin/env bash
set -e

# Create directory structure
mkdir -p iso/boot/grub

# Copy only the IncludeOS binary (no chainloader)
cp frequency_scaling_tool.elf.bin iso/boot/frequency_scaling_tool.elf.bin

# Create GRUB config for direct boot
cat > iso/boot/grub/grub.cfg << 'EOF'
set timeout=10
set default=0

menuentry "IncludeOS - Direct Boot (No Chainloader)" {
    multiboot /boot/frequency_scaling_tool.elf.bin
}

EOF

# Create bootable ISO
grub-mkrescue --modules="multiboot iso9660 configfile normal" -o frequency_scaling_tool.iso iso

echo "ISO created: frequency_scaling_tool.iso"
echo "Files in ISO structure:"
find iso/ -type f

# Clean up temporary ISO structure
rm -rf iso
