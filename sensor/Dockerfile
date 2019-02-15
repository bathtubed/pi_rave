FROM balenalib/raspberry-pi-debian-python

COPY ./firmware/hardfp/opt/vc /opt

RUN apt-get update && \
	apt-get install -yq build-essential git \
		libmosquittopp-dev libmosquitto-dev fftw-dev && \
	apt-get clean && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/app

RUN git clone git://git.drogon.net/wiringPi && \
	cd wiringPi && \
	git pull origin && \
	WIRINGPI_SUDO=env ./build && \
	cd ..


COPY start.sh ./start.sh

COPY ./sense ./sense

RUN cd sense && make install && make clean && cd ..

ENV INITSYSTEM on
CMD ["bash", "start.sh"]
