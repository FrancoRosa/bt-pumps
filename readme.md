# board game

> Programming game with real objects

This repo contains the required software to a control a rover from a board by using a bluetooth bridge

## Dependencies

Install arduino-cli

```bash
$ curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh
```

Add the arduino-cli to path

```bash
echo 'export PATH="$HOME/bin:$PATH"' >> ~/.bashrc
```

Install required boards

```bash
arduino-cli core install arduino:avr
```

Install required libraries

```bash
arduino-cli lib install RTClib
```
