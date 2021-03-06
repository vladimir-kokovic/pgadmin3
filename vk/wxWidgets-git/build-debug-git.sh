#!/bin/bash

set -v
set -e

#  --enable-stl            pgadmin don't like --enable-stl
#  --enable-metafiles
#  --enable-extended_rtti

rm -rf debug-git
mkdir -p debug-git
cd debug-git
export CXXFLAGS="-DwxUSE_FUNC_TEMPLATE_POINTER=0"
../wxWidgets/configure --srcdir=../wxWidgets --prefix=/mnt/sdd1/opt/wxWidgets-debug-git \
  --enable-compat28      \
  --enable-plugins       \
  --enable-debug_gdb     \
  --enable-std_iostreams \
  --enable-std_string    \
  --enable-utf8          \
  --enable-permissive    \
  --enable-intl          \
  --enable-xlocale       \
  --enable-config        \
  --enable-protocols     \
  --enable-ftp           \
  --enable-http          \
  --enable-fileproto     \
  --enable-sockets       \
  --enable-ipv6          \
  --enable-dataobj       \
  --enable-ipc           \
  --enable-baseevtloop   \
  --enable-epollloop     \
  --enable-selectloop    \
  --enable-any           \
  --enable-apple_ieee    \
  --enable-arcstream     \
  --enable-base64        \
  --enable-backtrace     \
  --enable-catch_segvs   \
  --enable-cmdline       \
  --enable-datetime      \
  --enable-debugreport   \
  --enable-dialupman     \
  --enable-dynlib        \
  --enable-dynamicloader \
  --enable-exceptions    \
  --enable-ffile         \
  --enable-file          \
  --enable-filehistory   \
  --enable-filesystem    \
  --enable-fontenum      \
  --enable-fontmap       \
  --enable-fs_archive    \
  --enable-fs_inet       \
  --enable-fs_zip        \
  --enable-fswatcher     \
  --enable-geometry      \
  --enable-log           \
  --enable-longlong      \
  --enable-mimetype      \
  --enable-printfposparam\
  --enable-snglinst      \
  --enable-sound         \
  --enable-stdpaths      \
  --enable-stopwatch     \
  --enable-streams       \
  --enable-sysoptions    \
  --enable-tarstream     \
  --enable-textbuf       \
  --enable-textfile      \
  --enable-timer         \
  --enable-variant       \
  --enable-zipstream     \
  --enable-url           \
  --enable-protocol      \
  --enable-protocol-http \
  --enable-protocol-ftp  \
  --enable-protocol-file \
  --enable-threads       \
  --enable-docview       \
  --enable-help          \
  --enable-html          \
  --enable-htmlhelp      \
  --enable-xrc           \
  --enable-aui           \
  --enable-propgrid      \
  --enable-ribbon        \
  --enable-stc           \
  --enable-constraints   \
  --enable-loggui        \
  --enable-logwin        \
  --enable-logdialog     \
  --enable-mdi           \
  --enable-mdidoc        \
  --enable-mediactrl     \
  --enable-richtext      \
  --enable-postscript    \
  --enable-printarch     \
  --enable-svg           \
  --enable-graphics_ctx  \
  --enable-clipboard     \
  --enable-dnd           \
  --enable-accel         \
  --enable-animatectrl   \
  --enable-bmpbutton     \
  --enable-bmpcombobox   \
  --enable-button        \
  --enable-calendar      \
  --enable-caret         \
  --enable-checkbox      \
  --enable-checklst      \
  --enable-choice        \
  --enable-choicebook    \
  --enable-collpane      \
  --enable-colourpicker  \
  --enable-combobox      \
  --enable-comboctrl     \
  --enable-dataviewctrl  \
  --enable-datepick      \
  --enable-detect_sm     \
  --enable-dirpicker     \
  --enable-display       \
  --enable-editablebox   \
  --enable-filectrl      \
  --enable-filepicker    \
  --enable-fontpicker    \
  --enable-gauge         \
  --enable-grid          \
  --enable-headerctrl    \
  --enable-hyperlink     \
  --enable-imaglist      \
  --enable-infobar       \
  --enable-listbook      \
  --enable-listbox       \
  --enable-listctrl      \
  --enable-notebook      \
  --enable-notifmsg      \
  --enable-odcombobox    \
  --enable-popupwin      \
  --enable-radiobox      \
  --enable-radiobtn      \
  --enable-rearrangectrl \
  --enable-sash          \
  --enable-scrollbar     \
  --enable-searchctrl    \
  --enable-slider        \
  --enable-spinbtn       \
  --enable-spinctrl      \
  --enable-splitter      \
  --enable-statbmp       \
  --enable-statbox       \
  --enable-statline      \
  --enable-stattext      \
  --enable-statusbar     \
  --enable-taskbaricon   \
  --enable-tbarnative    \
  --enable-textctrl      \
  --enable-tipwindow     \
  --enable-togglebtn     \
  --enable-toolbar       \
  --enable-toolbook      \
  --enable-treebook      \
  --enable-treectrl      \
  --enable-commondlg     \
  --enable-aboutdlg      \
  --enable-choicedlg     \
  --enable-coldlg        \
  --enable-filedlg       \
  --enable-finddlg       \
  --enable-fontdlg       \
  --enable-dirdlg        \
  --enable-msgdlg        \
  --enable-numberdlg     \
  --enable-splash        \
  --enable-textdlg       \
  --enable-tipdlg        \
  --enable-progressdlg   \
  --enable-wizarddlg     \
  --enable-menus         \
  --enable-miniframe     \
  --enable-tooltips      \
  --enable-splines       \
  --enable-mousewheel    \
  --enable-validators    \
  --enable-busyinfo      \
  --enable-joystick      \
  --enable-dragimage     \
  --enable-palette       \
  --enable-image         \
  --enable-gif           \
  --enable-pcx           \
  --enable-tga           \
  --enable-iff           \
  --enable-pnm           \
  --enable-xpm           \
  --enable-ico_cur       \
  --enable-autoidman     \
  --disable-precomp-headers \
  --with-libpng          \
  --with-libjpeg         \
  --with-libtiff         \
  --with-libxpm          \
  --with-libiconv        \
  --with-gnomevfs        \
  --with-opengl          \
  --with-sdl             \
  --with-regex           \
  --with-zlib            \
  --with-expat           > configure-out1-git-debug.log 2>&1
make -j4 > make-out1-git-debug.log 2>&1
make install > make-install-out1-git-debug.log 2>&1

exit 0
