FROM ubuntu:22.04
RUN apt-get update -y
RUN apt-get install wget build-essential libtool autotools-dev automake pkg-config bsdmainutils python3 libevent-dev libboost-dev libsqlite3-dev libminiupnpc-dev libnatpmp-dev libzmq3-dev systemtap-sdt-dev unzip git -y
WORKDIR /opt/
RUN git clone https://github.com/GriffionProject/Griffion.git
RUN cd Griffion
RUN autogen.sh
RUN configure
RUN make check
RUN mv src/griffiond /usr/bin
RUN mv src/griffion-cli /usr/bin
#RUN wget https://github.com/GriffionProject/Griffion/releases/download/v27.02/griffion-v27.2-ubuntu-22.04.zip
#RUN unzip griffion-v27.2-ubuntu-22.04.zip
#RUN mv griffion-v27.2-ubuntu-22.04/bin/griffiond /usr/bin
#RUN mv griffion-v27.2-ubuntu-22.04/bin/griffion-cli /usr/bin
#RUN rm -R griffion-v27.2-ubuntu-22.04
#RUN rm griffion-v27.2-ubuntu-22.04.zip
RUN wget https://raw.githubusercontent.com/TheRetroMike/rmt-nomp/master/scripts/blocknotify.c
RUN gcc blocknotify.c -o /usr/bin/blocknotify
CMD /usr/bin/griffiond -printtoconsole
