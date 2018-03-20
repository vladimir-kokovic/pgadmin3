#!/bin/bash

set -v
set -e

POSTGRESQL=/mnt/sdd1/home/src/postgresql-devel
BUILD=dev-build

cd $POSTGRESQL
rm -rf $BUILD
mkdir $BUILD
chown postgres:postgres $BUILD

cd $POSTGRESQL/$BUILD
su -c "$POSTGRESQL/dev-build-postgres.sh" postgres

exit 0
