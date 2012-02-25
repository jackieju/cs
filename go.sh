#/bin/sh
make $1 && cd bin && ./mse -sp ../test:../lib/ test
