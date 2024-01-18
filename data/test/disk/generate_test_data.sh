dd if=/dev/zero of=good_disk_64b.vdisk bs=262144 count=1

dd if=/dev/zero of=bad_disk1.vdisk bs=262133 count=1

disk_name="disk_test_read1.vdisk "
dd if=/dev/zero of=$disk_name bs=135168 count=1 # 33 blocks
echo -en "test string" | dd of=$disk_name bs=1 seek=666 count=1 conv=notrunc #writes things to the disk at position "seek"
printf '\xde\xad\xbe\xef' | dd of=$disk_name bs=1 seek=0 count=1 conv=notrunc 
printf '\xff\xaa\x23\x5e' | dd of=$disk_name bs=1 seek=132000 count=1 conv=notrunc
