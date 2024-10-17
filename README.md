# Fireplace
A cozy fireplace in your terminal

![A gif of what to expect](demo.gif?raw=true "Cozy")

## Build and Run

* Install ncurses
    * Including the development packages (usually called libncurses5-dev or something similar)
* `make`
* `./fireplace`

### Options
```
Usage: ./fireplace [options]
        -c character    An ASCII character to draw the flames. Default is '@'.
        -h              Print this message.
        -f framerate    Set the framerate in frames/sec. Default is 20.
                        A framerate of zero will make frames spit out as soon as they are ready.
        -t temp         Set the maximum temperature of the flames. Default is 10.
                        A higher temp means taller flames. Press the up/down arrows
                        to change the temperature at any time.
        -r              print random characters

Press ^C or q at any time to douse the flames.^C
```

## Docker build and run
* `docker build . -t fireplace:latest`
* `docker run -it --rm fireplace`

### Run with arguments:
* `docker run -it --rm fireplace -t 7`
