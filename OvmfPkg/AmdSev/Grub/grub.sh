GRUB_MODULES="
	    part_msdos
	    part_gpt
	    cryptodisk
	    luks
	    gcry_rijndael
	    gcry_sha256
	    ext2
	    btrfs
	    xfs
	    fat
	    configfile
	    memdisk
	    sleep
	    normal
	    echo
	    test
	    regexp
	    linux
	    linuxefi
	    reboot
	    sevsecret
	    "
basedir=`dirname $0`
##
# different distributions have different names for grub-mkimage, so
# search all the known ones
##
for b in grub2-mkimage grub-mkimage; do
    if which $b > /dev/null 2>&1; then
	mkimage=$b
    fi
done
if [ -z "$mkimage" ]; then
    echo "Can't find grub mkimage"
    exit 1
fi

# GRUB's rescue parser doesn't understand 'if'.
echo 'normal (memdisk)/grub.cfg' >"${basedir}/grub-bootstrap.cfg"

# Now build a memdisk with the correct grub.cfg
rm -f ${basedir}/disk.fat
mkfs.msdos -C ${basedir}/disk.fat 64 || exit 1
mcopy -i ${basedir}/disk.fat ${basedir}/grub.cfg ::grub.cfg || exit 1


${mkimage} -O x86_64-efi -p '(crypto0)' -c ${basedir}/grub-bootstrap.cfg -m ${basedir}/disk.fat -o ${basedir}/grub.efi ${GRUB_MODULES} || exit 1

# remove the intermediates
for f in disk.fat grub-bootstrap.cfg; do
    rm -f ${basedir}/$f
done
echo "grub.efi generated in ${basedir}"
