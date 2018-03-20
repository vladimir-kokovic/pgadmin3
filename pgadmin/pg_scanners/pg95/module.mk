#######################################################################
#
# pgAdmin III - PostgreSQL Tools
#
# Copyright (C) 2002 - 2012, The pgAdmin Development Team
# This software is released under the PostgreSQL Licence
#
# module.mk - pgadmin/pg_scanners/pg95/ Makefile fragment
#
#######################################################################

scandir = pg_scanners/pg95

PG_INCLUDE=`${PG_CONFIG} --includedir`
PG_SVRINCLUDE=`${PG_CONFIG} --includedir-server`

MYCC = $(CC) -c $(CFLAGS) -DSCANNER_VERSION=95 -DPGADMIN_SCANNER \
-I$(srcdir)/$(scandir) \
-I$(srcdir)/include \
-I$(srcdir)/include/utils \
-I$(srcdir)/$(scandir)/src/backend/parser/95 \
-I$(srcdir)/$(scandir)/src/include \
-Wall -I$(PG_INCLUDE) -I$(PG_SVRINCLUDE)

pg95deps = \
$(scandir)/keywords.o \
$(scandir)/kwlookup.o \
$(scandir)/pgstrcasecmp.o \
$(scandir)/chklocale.o \
$(scandir)/wchar.o \
$(scandir)/mbutils.o \
$(scandir)/encnames.o \
$(scandir)/wstrncmp.o \
$(scandir)/scansup.o \
$(scandir)/pgadminScannerCommon.o

$(scandir)/$(am__dirstamp):
	@$(MKDIR_P) $(scandir)
	@: > $(scandir)/$(am__dirstamp)

$(scandir)/keywords.o: $(srcdir)/$(scandir)/src/backend/parser/keywords.c $(scandir)/$(am__dirstamp)
	$(MYCC) -o $@ $<

$(scandir)/kwlookup.o: $(srcdir)/$(scandir)/src/backend/parser/kwlookup.c $(scandir)/$(am__dirstamp)
	$(MYCC) -o $@ $<

$(scandir)/pgstrcasecmp.o: $(srcdir)/$(scandir)/src/port/pgstrcasecmp.c $(scandir)/$(am__dirstamp)
	$(MYCC) -o $@ $<

$(scandir)/chklocale.o: $(srcdir)/$(scandir)/src/port/chklocale.c $(scandir)/$(am__dirstamp)
	$(MYCC) -o $@ $<

$(scandir)/wchar.o: $(srcdir)/$(scandir)/wchar.c $(scandir)/$(am__dirstamp)
	$(MYCC) -o $@ $<

$(scandir)/mbutils.o: $(srcdir)/$(scandir)/mbutils.c $(scandir)/$(am__dirstamp)
	$(MYCC) -o $@ $<

$(scandir)/encnames.o: $(srcdir)/$(scandir)/src/backend/utils/mb/encnames.c $(scandir)/$(am__dirstamp)
	$(MYCC) -DFRONTEND -o $@ $<

$(scandir)/wstrncmp.o: $(srcdir)/$(scandir)/src/backend/utils/mb/wstrncmp.c $(scandir)/$(am__dirstamp)
	$(MYCC) -DFRONTEND -o $@ $<

$(scandir)/scansup.o: $(srcdir)/$(scandir)/scansup.c $(scandir)/$(am__dirstamp)
	$(MYCC) -o $@ $<

$(scandir)/pgadminScannerCommon.o: $(srcdir)/$(scandir)/pgadminScannerCommon.c $(scandir)/$(am__dirstamp)
	$(MYCC) -DXVER=0x95 -o $@ $<

$(scandir)/scan95.o: $(srcdir)/$(scandir)/scan.c $(pg3deps) $(scandir)/$(am__dirstamp)
	$(MYCC) -DXVER=0x95 -o $@ $<

$(scandir)/pgadminScanner95.o: $(srcdir)/$(scandir)/pgadminScanner.c $(scandir)/$(am__dirstamp)
	$(MYCC) -DXVER=0x95 -o $@ $<

EXTRA_DIST += \
	$(scandir)/module.mk \
	$(scandir)/scan.l
