#! /bin/sh

# Warning: this test requires bonobo-activation-server to not be running.


BONOBO_ACTIVATION_SERVER="../activation-server/bonobo-activation-server";
BONOBO_ACTIVATION_PATH=".:$BONOBO_ACTIVATION_PATH";
PATH=".:$PATH";
LD_LIBRARY_PATH="./.libs:$LD_LIBRARY_PATH";

export BONOBO_ACTIVATION_SERVER BONOBO_ACTIVATION_PATH PATH LD_LIBRARY_PATH

#if test ! -z "$(ps x | grep bonobo-activation-server)"; then
#    echo "WARNING: there seems to be already a bonobo-activation-server running, therefore this test might fail";
#fi

# job control must be active
set -m

echo "Starting factory"
./generic-factory > generic-factory.output &
sleep 1

echo "Starting client"
./test-generic-factory > test-generic-factory.output

echo "Waiting for factory to terminate; Please hold on a second, otherwise hit Ctrl-C."
wait %1

echo "Comparing factory output with model..."
if diff -u models/generic-factory.output generic-factory.output; then
    echo "...OK"
    rm -f generic-factory.output
else
    echo "...DIFFERENT!"
    rm -f generic-factory.output
    rm -f test-generic-factory.output
    exit 1;
fi

echo "Comparing client output with model..."
if diff -u models/test-generic-factory.output test-generic-factory.output; then
    echo "...OK"
    rm -f test-generic-factory.output
else
    echo "...DIFFERENT!"
    rm -f test-generic-factory.output
    exit 1;
fi

exit 0

