
message() {
	echo -e '\E[1;33m'$1'\E[0m'	
}



compile_dts() {
        rm -rf arch/arm64/boot/dts/freescale/$1.dtb
        make ARCH=arm64 CROSS_COMPILE=aarch64-poky-linux- freescale/$1.dtb
        
        if [ -z arch/arm64/boot/dts/freescale/$1.dtb ]; then
        	message "DTB missing!"
        	exit
        fi
        
        cp arch/arm64/boot/dts/freescale/$1.dtb $2
} 



createmodules() { 
	rm -rf ./build/modules;

 	rm -rf $OUTPUTDIR/modules_$BUILDVER/*
	rm -rf $OUTPUTDIR/modules_$BUILDVER.tar.gz

 	make ARCH=arm64 CROSS_COMPILE=aarch64-poky-linux- -j8 modules INSTALL_MOD_PATH=./build/modules
 	make ARCH=arm64 CROSS_COMPILE=aarch64-poky-linux- -j8 modules_install INSTALL_MOD_PATH=./build/modules

        pushd ./build/modules/
	tar cvfz modules_$BUILDVER.tar.gz lib/
	popd

        mv ./build/modules/modules_$BUILDVER.tar.gz $OUTPUTDIR

	message "eNOBU modules created."
}

cptonobu() {
	message "copying kernel and nobu.dtb to tftp.."
	cp -v $OUTPUTDIR/*.dtb /srv/tftpboot
        cp -v $OUTPUTDIR/Image /srv/tftpboot
	cp -v $OUTPUTDIR/modules_$BUILDVER.tar.gz /srv/tftpboot;
}




message "Building eNOBU kernel"
CFG=imx8_enobu_defconfig     
BUILD_EXT=""

make ARCH=arm64 CROSS_COMPILE=aarch64-poky-linux- $CFG

# BUILDVER=$(cat .config | grep LOCALVERSION | awk -F'"' '{print substr($2,2)}')

BUILDVER="001"

OUTPUTDIR=build/$BUILDVER$BUILD_EXT
rm -rf $OUTPUTDIR
mkdir -p $OUTPUTDIR

rm -rf arch/arm64/boot/Image.gz

make ARCH=arm64 CROSS_COMPILE=aarch64-poky-linux- -j8
if [ -z arch/arm64/boot/Image.gz ]; then
	message "Image.gz missing!"
	exit
fi

cp arch/arm64/boot/Image.gz $OUTPUTDIR

compile_dts fsl-imx8mq-enobu-emmc-wifi-hdmi $OUTPUTDIR

# compile all dtbs
# compile_dts nobu $OUTPUTDIR
# compile_dts nobu-poe-exp $OUTPUTDIR
# compile_dts nobu-dsa-switch $OUTPUTDIR
# compile_dts nobu-dsa-switch-12ports $OUTPUTDIR
# compile_dts nobu-dsa-gigaswitch $OUTPUTDIR

createmodules

# cptonobu

message "Output folder: $OUTPUTDIR"
ls -la $OUTPUTDIR

