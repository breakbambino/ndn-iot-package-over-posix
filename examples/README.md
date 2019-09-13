# Instruction of examples
This directory contains several basic examples to show how to use ndn-lite in posix systems. Specifically, it provides the following well-tested examples: <br>
## Deamon producer
The source file of this example is [deamon-producer.c](https://github.com/9th-ndn-hackathon/ndn-iot-package-over-posix/blob/master/examples/deamon-producer.c). The example runs as a data producer, which uses the ndn-lie forwarder and construct an unix sockets face for transmiting the data, which is a simple string: "I'm a Data packet.". It requires use input the `name prefix` of the generated data as the argument. Deamon producer will respnond to the Interest with the same name prefix from the other requestor. <br>
## File transfer
The source files of this example are [file-transfer-client.c](https://github.com/9th-ndn-hackathon/ndn-iot-package-over-posix/blob/master/examples/file-transfer-client.c) and [file-transfer-server.c](https://github.com/9th-ndn-hackathon/ndn-iot-package-over-posix/blob/master/examples/file-transfer-server.c). The example is a typical client-server file transmission application. It utilizes the ndn-lite's forwarder and UDP face to tansmit the data. At server end, it requires four arguments: `<local-port>`, which refers to the port used in UDP; `<client-ip>`, which refers to the IP address of the client;  `<client-port>`, which refers to the port used in the client;  `<name-prefix>`, which refers to the name prefix of the data. Similarly, the client requires the arguments in terms of `<local-port>`,  `<remote-ip>`, `<remote-port>`, `<name-prefix>`, and `<file-name>`. `<remote-ip>` and `<remote-port>` indicate the IP address and port of the server. User  can use `<file-name>` to name the received file.
## NDN putchunk
The source file of this example is [ndn-putchunks.c](https://github.com/9th-ndn-hackathon/ndn-iot-package-over-posix/blob/master/examples/ndn-putchunks.c). This example is similar to the example of ndn putchunk in NDN-CXX tools, which is used to respond to the request for a chunk of data. Instead of using NFD, it utilizes ndn-lite's forwarder and unix socket face within it.
## Basic consumer and producer using NFD
The source files of this example are [nfd-basic-consumer.c](https://github.com/9th-ndn-hackathon/ndn-iot-package-over-posix/blob/master/examples/nfd-basic-consumer.c) and [nfd-basic-producer.c](https://github.com/9th-ndn-hackathon/ndn-iot-package-over-posix/blob/master/examples/nfd-basic-producer.c). The example is a typical consumer-producer demo by using NFD as the forwarder. Specifically, it utilizes the unix sockets face to transmist data so that the developer can try to transmis between the local ndn-lite forwarder and NFD. Both two ends must specify the `<name-prefix>`, which is the name prefix of the data: "I'm a Data packet.".
## Basic consumer and producer over unicast and mulitcast UDP
### Unicast example
The source files of this example are [udp-basic-consumer.c](https://github.com/9th-ndn-hackathon/ndn-iot-package-over-posix/blob/master/examples/udp-basic-consumer.c) and [udp-basic-producer.c](https://github.com/9th-ndn-hackathon/ndn-iot-package-over-posix/blob/master/examples/udp-basic-producer.c).  The example is a typical consumer-producer demo using ndn-lite's forwarder and unicast UDP face to tansmit the data. At producer end, it requires four arguments: `<local-port>`, which refers to the port used in UDP; `<remote-ip>`, which refers to the IP address of the consumer;  `<remote-port>`, which refers to the port used in the consumer;  `<name-prefix>`, which refers to the name prefix of the data. Similarly, the consumer requires the same arguments while the `<remote-ip>` and `<remote-port>` indicate the IP address and port of the produceer.
### Multicast example
The source files of this example are [udp-group-consumer.c](https://github.com/9th-ndn-hackathon/ndn-iot-package-over-posix/blob/master/examples/udp-group-consumer.c) and [udp-group-producer.c](https://github.com/9th-ndn-hackathon/ndn-iot-package-over-posix/blob/master/examples/udp-group-producer.c).  The example is a typical consumer-producer demo using ndn-lite's forwarder and multicast UDP face to tansmit the data. Specifically, the developer should specicy `<name-prefix>`, which refers to the name prefix of the data (string "I'm a Data packet."). The exmaple uses the port `56363` and group ip `224.0.23.170`, which can be modified in the source code:
```C
 uint32_t portnum = 56363;
 port = htons(portnum);
 server_ip = inet_addr("224.0.23.170");
 ```