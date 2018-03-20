#!/bin/bash

set -v
set -e

POSTGRESQL=/mnt/sdd1/home/src/postgresql-devel
BUILD=dev-build

cd $POSTGRESQL/$BUILD

export CFLAGS="-g3 -gdwarf-2"

$POSTGRESQL/postgresql-git/postgresql/configure "--srcdir=$POSTGRESQL/postgresql-git/postgresql" '--enable-cassert' \
'--enable-nls' '--enable-integer-datetimes' '--with-perl' '--with-python' '--with-tcl' '--with-openssl' \
'--enable-thread-safety' '--with-ldap' '--with-gssapi' '--with-pam' '--with-libxml' '--with-libxslt' \
"--prefix=$POSTGRESQL/dev-install" > configure-out-dev.log 2>&1

make world > make-out-dev.log 2>&1

make install-world > make-install-out-dev.log 2>&1

exit 0
