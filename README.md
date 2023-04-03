# pzem-004t-v3-emulator
This is Arduino emulator for pzem-004t v3 electricity meter. Can be used to troubleshoot issues or setup the dashboards while waiting for real module to arrive. It responds with pseudo random data which changes every request.
## Usage
1. Set correct Serial devices, upload to your arduino module.
2. Connect RX/TX to your ESP or Arduino reader.
3. Make sure ground is shared between modules.
4. Read module using any Modbus adress. It always responds with adress 1, you can change it in the code.
