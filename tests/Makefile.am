NULL =

noinst_PROGRAMS =				\
	coroutine				\
	util					\
	session					\
	$(NULL)

TESTS = $(noinst_PROGRAMS)

AM_CPPFLAGS =					\
	$(GIO_CFLAGS) -I$(top_srcdir)/gtk	\
	-DG_LOG_DOMAIN=\"GSpice\"		\
	$(NULL)

AM_LDFLAGS = $(GIO_LIBS) -static

LDADD =							\
	$(top_builddir)/gtk/libspice-client-glib-2.0.la	\
	$(NULL)

util_SOURCES = util.c
coroutine_SOURCES = coroutine.c
session_SOURCES = session.c

-include $(top_srcdir)/git.mk
