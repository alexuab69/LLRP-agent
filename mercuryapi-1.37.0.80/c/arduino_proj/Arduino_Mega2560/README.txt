Steps to run Read.ino.
-----------------------

1. Open Read.ino file (available in the Read folder) in Arduino IDE.
2. Select board in tools->Board->Arduino AVR Boards->Arduino Mega or Mega 2560.
3. Select comport in Tools->Port.
4. Give path of mercuryapi_src.zip file (available in the current folder) in Sketch->Include Library-> Add .ZIP Library.
5. Build and run. It will perform timed read for 500ms followed by continuous read of 500ms read time.

Please note that mercuryapi_src.zip contains C API source files of the version mercuryapi-BILBO-1.37.0.75.
