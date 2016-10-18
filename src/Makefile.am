plugindir = $(CLAWS_MAIL_PLUGINDIR)

plugin_LTLIBRARIES = gnome.la

gnome_la_SOURCES = \
	gnome_plugin.c \
	gettext.h

gnome_la_LDFLAGS = \
	-avoid-version -module \
	$(GTK_LIBS) \
	$(LIBSECRET_LIBS) \
	$(libebook_LIBS)

AM_CPPFLAGS = \
	-Wall \
	$(CLAWS_MAIL_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GTK_CFLAGS) \
	$(LIBSECRET_CFLAGS) \
	$(libebook_CFLAGS) \
	-DLOCALEDIR=\""$(localedir)"\"
