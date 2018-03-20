#!/bin/bash

set -v
set -e

rm -rf debug-wx-git
mkdir debug-wx-git

cd pgadmin3
bash bootstrap

cd ./pgadmin
./ver_svn.sh
cd ../../debug-wx-git

export CFLAGS="-gdwarf-2 -g3"
export CXXFLAGS="-gdwarf-2 -g3"
export LDFLAGS="-Xlinker --cref -Xlinker -Map -Xlinker /tmp/pgadmni3-wx-git.map"

../pgadmin3/configure --prefix=/usr/local/pgadmin3-debug-wx-git \
--enable-debug --enable-databasedesigner --srcdir=../pgadmin3 --with-pgsql=/mnt/sdd1/home/src/postgresql-devel/install \
--with-wx-version=3.1 --with-wx=/mnt/sdd1/opt/wxWidgets-debug-git > configure-out-debug-wx-git.log 2>&1

make -j4 > make-out-debug-wx-git.log 2>&1
make install > make-install-out-debug-wx-git.log 2>&1

exit 0
