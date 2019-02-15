FROM balenalib/raspberrypi3-debian-python

RUN apt-get && \
	apt-get install pip3 && \
	pip3 install rpi_ws281x paho-mqtt && \
	apt-get clean && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/app

ENV INITSYSTEM on

CMD ["bash", "python3 LED_control.py"]