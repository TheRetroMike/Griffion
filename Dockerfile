FROM ubuntu:22.04
RUN apt-get update -y
RUN apt-get install wget build-essential install libminiupnpc-dev libnatpmp-dev unzip -y
WORKDIR /opt/
RUN wget https://github.com/GriffionProject/Griffion/releases/download/v27.02/griffion-v27.2-ubuntu-22.04.zip
RUN unzip griffion-v27.2-ubuntu-22.04.zip
RUN mv griffion-v27.2-ubuntu-22.04/bin/griffiond /usr/bin
RUN mv griffion-v27.2-ubuntu-22.04/bin/griffion-cli /usr/bin
RUN rm -R griffion-v27.2-ubuntu-22.04
RUN rm griffion-v27.2-ubuntu-22.04.zip
RUN wget https://raw.githubusercontent.com/TheRetroMike/rmt-nomp/master/scripts/blocknotify.c
RUN gcc blocknotify.c -o /usr/bin/blocknotify
CMD /usr/bin/griffiond -printtoconsole
