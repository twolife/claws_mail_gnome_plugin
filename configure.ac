AC_PREREQ(2.60)

PACKAGE=gnome_plugin

dnl plugin version
MAJOR_VERSION=0
MINOR_VERSION=2
MICRO_VERSION=0

if test \( $MICRO_VERSION -eq 0 \); then
    VERSION=${MAJOR_VERSION}.${MINOR_VERSION} 
else
    VERSION=${MAJOR_VERSION}.${MINOR_VERSION}.${MICRO_VERSION}
fi

AC_INIT(src/gnome_plugin.c)
AM_MAINTAINER_MODE

AM_INIT_AUTOMAKE($PACKAGE, $VERSION, no-define)
AC_CONFIG_HEADERS(config.h)
AC_DEFINE_UNQUOTED(PLUGINVERSION, "$VERSION", [plugin version])

AC_PROG_CC
AC_ISC_POSIX
AM_PROG_CC_STDC
AC_PROG_INSTALL
AC_PROG_CPP
AC_PROG_LIBTOOL
AC_PROG_MKDIR_P

dnl Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT

CFLAGS="$CFLAGS -Wall"
if test $USE_MAINTAINER_MODE = yes; then
	CFLAGS="$CFLAGS -DGTK_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DG_DISABLE_DEPRECATED"
fi

dnl Find pkg-config
AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
if test x$PKG_CONFIG = xno ; then
  AC_MSG_ERROR([*** pkg-config not found. See http://www.freedesktop.org/software/pkgconfig/])
fi

if test -z $prefix || test "${prefix}" = "NONE" ; then
  prefix=$( $PKG_CONFIG --variable=prefix claws-mail )
  CLAWS_MAIL_PLUGINDIR=$( $PKG_CONFIG --variable=plugindir claws-mail )
else
  CLAWS_MAIL_PLUGINDIR='${libdir}/claws-mail/plugins'
fi

dnl Check for claws-mail
PKG_CHECK_MODULES(CLAWS_MAIL, claws-mail >= 3.9.3)
AC_SUBST(CLAWS_MAIL_CFLAGS)
AC_SUBST(CLAWS_MAIL_LIBS)
AC_SUBST(CLAWS_MAIL_PLUGINDIR)

dnl Check for GLib
PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.6 gmodule-2.0 >= 2.6 gobject-2.0 >= 2.6 gthread-2.0 >= 2.6)

GLIB_GENMARSHAL=`pkg-config --variable=glib_genmarshal glib-2.0`
AC_SUBST(GLIB_GENMARSHAL)
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

dnl Check for GTK+
PKG_CHECK_MODULES(GTK, gtk+-2.0 >= 2.6)

dnl EDS support via libebook
LIBEBOOK_REQUIRED=3.6
PKG_CHECK_MODULES(libebook, libebook-1.2 >= $LIBEBOOK_REQUIRED)
AC_SUBST(libebook_CFLAGS)
AC_SUBST(libebook_LIBS)

dnl libsecret, library for storing and retrieving passwords
PKG_CHECK_MODULES([LIBSECRET], [libsecret-1])
AC_SUBST(LIBSECRET_CFLAGS)
AC_SUBST(LIBSECRET_LIBS)

AC_SUBST(PLUGINVERSION)

AC_OUTPUT([
Makefile
src/Makefile
])
