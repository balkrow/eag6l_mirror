#dd if=eag6l_fpga_top_20240827_f10.pof of=eag6l_fpga_top_20240827_f10.bin bs=1 skip=2097295 count=716800
FPGA_DIR=$TOPDIR/fpga
if ! [ -f "$FPGA_DIR/$FPGA_POF_FILE" ]; then 
  echo "FPGA pof file $FPGA_DIR/$FPGA_POF_FILE not found" 
  exit 1
else
rm $FPGA_DIR/eag6l_fpga_v${FPGA_VER}.bin
echo "$FPGA_DIR/$FPGA_POF_FILE ->  $FPGA_DIR/eag6l_fpga_v${FPGA_VER}.bin" 
dd if=$FPGA_DIR/${FPGA_POF_FILE} of=$FPGA_DIR/eag6l_fpga_v${FPGA_VER}.bin bs=1 skip=2097295 count=$FPGA_IMG_SIZE
fi
