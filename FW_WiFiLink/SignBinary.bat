@set ESParduino=c:/Users/maticpi/AppData/Local/arduino15/packages/esp8266/hardware/esp8266/2.6.3/
@set PrivateKey=d:/Projects/Jubomer/FW_JubomerV2/private.key
@set binary=d:/Projects/Jubomer/FW_JubomerV2/Release/FW_JubomerV2.bin
@set path=%PATH%;c:\Users\maticpi\AppData\Local\arduino15\packages\esp8266\tools\python3\3.7.2-post1\

python signing.py --mode sign --privatekey %PrivateKey% --bin %binary% --out %binary%.signed

@set path=%PATH:;c:\Users\maticpi\AppData\Local\arduino15\packages\esp8266\tools\python3\3.7.2-post1\=%
