FROM balenalib/raspberrypi3-debian-python

RUN apt-get update && \
	apt-get install -yq build-essential libmosquittopp-dev libmosquitto-dev pip3 && \
	pip3 install rpi_ws281x paho-mqtt && \
	apt-get clean && rm -rf /var/lib/apt/lists/*


WORKDIR /usr/src/app

COPY . ./

ENV INITSYSTEM on

CMD ["bash"]
