dd if=/dev/zero of=good_disk_64b.vdisk bs=262144 count=1
dd if=/dev/zero of=bad_disk1.vdisk bs=262133 count=1
dd if=/dev/zero of=disk_test_read1.vdisk bs=135168 count=1 # 33 blocks
