#!/bin/bash

cd postgresql/
git diff --unified=3 --binary --check master > /tmp/pg.check
less /tmp/pg.check

exit 0
