# Device used for local testing
BALENA_MACHINE_NAME=raspberrypi3
APP_NAME=pi_rave
BALENA_CMD=balena

# Device name of the target
PI_ADDRESS=192.168.1.73

local-test:
	$(BALENA_CMD) push $(PI_ADDRESS)

deploy:
	$(BALENA_CMD) push $(APP_NAME)

clean:
	rm */Dockerfile
