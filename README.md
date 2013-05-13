#This does not work at the moment
##If anyone knows what is wrong, please email plpub2@gmail.com

The code currently compiles with the following linker errors:

drone_proj.cpp.o: In function `digi_batt::i2cWrite16(unsigned int, unsigned char)':
digi_write.ino:343: undefined reference to `Wire'
digi_write.ino:343: undefined reference to `Wire'
digi_write.ino:343: undefined reference to `TwoWire::beginTransmission(int)'
digi_write.ino:344: undefined reference to `TwoWire::write(unsigned char)'
digi_write.ino:345: undefined reference to `TwoWire::write(unsigned char)'
digi_write.ino:346: undefined reference to `TwoWire::write(unsigned char)'
digi_write.ino:347: undefined reference to `TwoWire::endTransmission()'
drone_proj.cpp.o: In function `digi_batt::init(digi_pins*, short, short)':
digi_write.ino:213: undefined reference to `Wire'
digi_write.ino:213: undefined reference to `Wire'
digi_write.ino:213: undefined reference to `TwoWire::begin()'

A cut-down version to show these errors more clearly is included in the directory "Problemtest"