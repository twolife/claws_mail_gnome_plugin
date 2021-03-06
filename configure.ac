AC_PREREQ(2.60)

AC_INIT([gnome_plugin], [0.2.1])
AC_CONFIG_SRCDIR([src/gnome_plugin.c])
AM_MAINTAINER_MODE

AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS(config.h)

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
PKG_PROG_PKG_CONFIG
if test -z "$PKG_CONFIG" ; then
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

GLIB_GENMARSHAL=$( $PKG_CONFIG --variable=glib_genmarshal glib-2.0 )
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

AC_OUTPUT([
Makefile
src/Makefile
])
