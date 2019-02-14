FROM balenalib/raspberrypi3-debian-python

RUN apt-get update && \
	apt-get install -yq build-essential libmosquittopp-dev libmosquitto-dev && \
	apt-get clean && rm -rf /var/lib/apt/lists/*


WORKDIR /usr/src/app

COPY . ./

ENV INITSYSTEM on

CMD ["bash"]
